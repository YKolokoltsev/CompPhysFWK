//
// Created by Dr, Yevgeniy Kolokoltsev on 1/30/17.
//

#ifndef COMPPHYSFWK_PIPELINE_HPP
#define COMPPHYSFWK_PIPELINE_HPP

#include <memory>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
#include <string>

using namespace std;

using t_p_cond = shared_ptr<condition_variable>;

enum class CMD_MSG{stop, none};
enum class QUEUE_POLICY{wait, drop};

struct MESSAGE{
    MESSAGE(){}
    MESSAGE(CMD_MSG cmd) : cmd{cmd} {}
    CMD_MSG cmd = CMD_MSG::none;
};

/*
 * The input queue of any filter contains pointers onto the messages of it's own, filter's custom
 * type 'T_IN'. It is possible to save some memory by upcasting queue type to MESSAGE,  however it
 * will require continuous and dangerous downcast of the working messages. In our case it is better
 * to loose a bit of memory in command messages, there is a little of them.
 */
template <typename T_IN>
shared_ptr<T_IN> cmd(CMD_MSG cmd){
    static_assert(is_base_of<MESSAGE,T_IN>(),"The T_MSG shell be derived from MESSAGE class");
    shared_ptr<T_IN> msg(new T_IN);
    msg->cmd = cmd;
    return msg;
}


//***************DUMMY THREAD WITH INPUT MESSAGE QUEUE***********************
/*
 * There is two basic usages of this class:
 * 1. There is no need to store any additional local state variables in the child process
 * and it shell have just an input queue of specified messages. This can be a statistics terminal
 * device or any other simple end-point. In this case we can use a second constructor and specify
 * a process_usr_msg pointer to a function.
 *
 * 2. It is possible to derive any functional type from this one extending/overwriting it functions with
 * something useful: IN-OUT type (filter),  stateful end-device/source etc...
 */
template<typename T_IN>
class BaseNode{
    static_assert(is_base_of<MESSAGE,T_IN>(),"The T_IN_MSG shell be derived from MESSAGE class");

public:
    using t_IN_PTR = shared_ptr<T_IN>;

    BaseNode(string name = ""): name{name}{}
    BaseNode(function<bool(t_IN_PTR&&)> func_process, string name=""): name{name},  func_process{func_process} {};

    virtual ~BaseNode(){
        // Each 'BaseNode' child is a node in some distributed network
        // if a process just disappear (even gracefully), the connectivity of the graph can be altered,
        // making it impossible to run wave algorithms, such as an explicit termination.
        // So by default we start a "stop" propagation wave over all the network and therefore
        // kill it. In special cases this destructor can be overwritten.

        // Join with a thread that called destructor
        stop();
    }

    //bool is_running(){return v_running;}

    void start(){
        own_thread = thread(run,this);
        //todo: remove this add hook
        while(!v_running) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void stop(){
        if(!v_running) return;
        put(cmd<T_IN>(CMD_MSG::stop),QUEUE_POLICY::wait);
        own_thread.join();
    }

    bool put(t_IN_PTR&& val, QUEUE_POLICY pol = QUEUE_POLICY::drop){
        //No messages can be accepted until this node has a working
        //message thread. This is just a safe solution. Probably can be eliminated
        //after implementation of various todo
        if(!v_running) return false;

        //lock a local state for inter-thread communication
        unique_lock<mutex> lck(local_state_mtx);

        //input queue is full
        if(in.size() > 10){
            if(pol == QUEUE_POLICY::wait){
                //unlock local state and wait until the queue shortens
                //when the event arrive local state will be relocked
                event.wait(lck,[this]{return in.size() <= 10;});
            }else if(pol == QUEUE_POLICY::drop){
                //the message was sent via rvalue, so it is dropped if not stored
                //in this case the message shared_ptr<> destructor is called
                return false;
            }
        }

        //push the message, notify local thread if it was blocked and
        //confirm received by returning true
        in.push_back(move(val));
        event.notify_one();
        //'lck' destructor unlocks 'local_state_mtx' automatically
        return true;
    }

protected:
    //We have two basic usages of this class. In the case when process
    // Any child can overwrite this function it is
    virtual void main(){
        t_IN_PTR curr_in;
        while(1){
            curr_in = pull_msg();
            //todo: implement a clear termination algorithm here
            if(curr_in->cmd == CMD_MSG::stop) break;
            if(!process_usr_msg(move(curr_in))){
                cerr << "warning: process_usr_msg failed, send 'stop' if critical but return 'true'" << endl;
            }
        }
    }
    virtual bool process_usr_msg(t_IN_PTR&& msg){
        if(!func_process){
            cerr << "warning: user function not specified nor overloaded, pointless but possible" << endl;
            return true;
        }else{
            return func_process(move(msg));
        }
    };

    t_IN_PTR pull_msg(bool wait = true){
        unique_lock<mutex> lck(local_state_mtx);
        if(wait){
            event.wait(lck,[=]{return !in.empty();});
        }else if(in.empty()){
            return nullptr;
        }
        t_IN_PTR curr_in = in.front();
        in.pop_front();
        event.notify_one();
        return curr_in;
    }

private:
    //static function can't be virtual, so it is a dummy wrapper for virtual implementation
    //this approach permits override loop body in child class if necessary (this is the case for different child sources)
    //resulting design is also a bit faster because here we move directly to class address space
    static void run(BaseNode* f){
        f->v_running = true;
        f->main();
        f->v_running = false;
    };

protected:
    string name;
    mutex local_state_mtx;
    //todo: user function to be called in a simple end-point node
    function<bool(t_IN_PTR&&)> func_process{nullptr};

private:
    thread own_thread;
    list<t_IN_PTR> in;
    condition_variable event;
    bool v_running = false;
};

//****************************SOURCE CLASS***********************

template<typename T_OUT>
class source : public BaseNode<MESSAGE>{
    static_assert(is_base_of<MESSAGE,T_OUT>(),"T2 shell be derived from MESSAGE class");
public:
    using tBase = BaseNode<MESSAGE>;
    using t_OUT_PTR = shared_ptr<T_OUT>;
    using t_IN_PTR = tBase::t_IN_PTR;
    source(function<t_OUT_PTR()> func_acquire, QUEUE_POLICY pol = QUEUE_POLICY::drop, string name = ""):
            func_acquire{func_acquire}, tBase(name), pol(pol) {}

    void set_target(shared_ptr<BaseNode<T_OUT>> target){next = target;}

protected:
    virtual void main(){
        tBase::main();
    }
    virtual bool process_usr_msg(t_IN_PTR && msg){
        if(next){
            next->put(func_acquire(), pol);
        }else{
            cerr << "broken pipe detected" << endl;
            return false;
        }
        return true;
    };
protected:
    QUEUE_POLICY pol;
    function<t_OUT_PTR()> func_acquire;
    shared_ptr<BaseNode<T_OUT>> next{nullptr};
};

//************************FILTER***********************
template<typename T_IN, typename T_OUT>
class filter : public BaseNode<T_IN>{
    static_assert(is_base_of<MESSAGE,T_OUT>(),"T2 shell be derived from MESSAGE class");

public:
    using tBase = BaseNode<T_IN>;
    using t_IN_PTR = shared_ptr<T_IN>;
    using t_OUT_PTR = shared_ptr<T_OUT>;

    filter(function<t_OUT_PTR(t_IN_PTR&&)> func_filter, QUEUE_POLICY pol = QUEUE_POLICY::drop, string name = ""):
            func_filter(func_filter), BaseNode<T_IN>(name), pol(pol) {}

    void set_target(shared_ptr<BaseNode<T_OUT>> target){next = target;}

protected:
    virtual bool process_usr_msg(t_IN_PTR&& msg){
        if(next){
            return next->put(func_filter(move(msg)), pol);
        }else{
            cerr << "broken pipe detected" << endl;
            return false;
        }
    };

private:
    QUEUE_POLICY pol;
    shared_ptr<BaseNode<T_OUT>> next;
    function<t_OUT_PTR(t_IN_PTR&&)> func_filter;
};

#endif //COMPPHYSFWK_PIPELINE_HPP
