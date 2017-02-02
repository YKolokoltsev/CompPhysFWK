//
// Created by morrigan on 1/30/17.
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

template <typename T_MSG>
shared_ptr<T_MSG> cmd(CMD_MSG cmd){
    static_assert(is_base_of<MESSAGE,T_MSG>(),"The T_MSG shell be derived from MESSAGE class");
    shared_ptr<T_MSG> msg(new T_MSG);
    msg->cmd = cmd;
    return msg;
}


//***************DUMMY THREAD WITH INPUT MESSAGE QUEUE***********************
template<typename T_IN>
class msg_thread{
    static_assert(is_base_of<MESSAGE,T_IN>(),"The T_IN_MSG shell be derived from MESSAGE class");

public:
    using t_IN_PTR = shared_ptr<T_IN>;

    msg_thread(QUEUE_POLICY pol, string name = ""): pol{pol}, name{name}{
    }
    ~msg_thread(){stop();}

    void join(){ own_thread.join();}

    bool is_running(){return v_running;}

    void start(){
        own_thread = thread(run,this);
        //todo: remove this add hook
        while(!is_running()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void stop(){
        if(!v_running) return;
        put(cmd<T_IN>(CMD_MSG::stop),QUEUE_POLICY::wait);
        own_thread.join();
    }

    bool put(t_IN_PTR&& val, QUEUE_POLICY pol = QUEUE_POLICY::drop){
        //lock local state
        unique_lock<mutex> lck(local_state_mtx);

        //broken pipe
        if(!is_running()) return false;

        //input queue is full
        if(in.size() > 10){
            if(pol == QUEUE_POLICY::wait){
                //unlock local state and wait until the queue shortens
                //when the event arrive local state will be relocked
                event.wait(lck,[this]{return in.size() <= 10;});
            }else if(pol == QUEUE_POLICY::drop){
                //the message was sent via rvalue, so it is dropped if not stored
                //in this case shared_ptr<> destructor is called
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
    virtual void main(){
        t_IN_PTR curr_in;
        while(1){
            curr_in = pull_msg();
            if(curr_in->cmd == CMD_MSG::stop) break;
            if(!process_usr_msg(move(curr_in))){ cerr << "usr msg failed (data may be lost)" << endl; }
        }
    }
    virtual bool process_usr_msg(t_IN_PTR&& msg){cerr << "warning: not overloaded!!!" << endl; return false;};

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
    static void run(msg_thread* f){
        f->v_running = true;
        f->main();
        f->v_running = false;
    };

protected:
    QUEUE_POLICY pol;
    string name;
    mutex local_state_mtx;

private:
    thread own_thread;
    list<t_IN_PTR> in;
    condition_variable event;
    bool v_running = false;
};

//****************************SOURCE CLASS***********************

template<typename T_OUT>
class source : public msg_thread<MESSAGE>{
    static_assert(is_base_of<MESSAGE,T_OUT>(),"T2 shell be derived from MESSAGE class");
public:
    using tBase = msg_thread<MESSAGE>;
    using t_OUT_PTR = shared_ptr<T_OUT>;
    using t_IN_PTR = tBase::t_IN_PTR;
    source(function<t_OUT_PTR()> func_acquire, QUEUE_POLICY pol = QUEUE_POLICY::drop, string name = ""):
            func_acquire{func_acquire}, tBase(pol, name){}

    void set_target(shared_ptr<msg_thread<T_OUT>> target){next = target;}

protected:
    virtual void main(){
        tBase::main();
    }
    virtual bool process_usr_msg(t_IN_PTR && msg){
        if(next){
            next->put(func_acquire(), tBase::pol);
        }else{
            cerr << "broken pipe detected" << endl;
            return false;
        }
        return true;
    };
protected:
    function<t_OUT_PTR()> func_acquire;
    shared_ptr<msg_thread<T_OUT>> next{nullptr};
};

//*************************DEVICE CLASS***********************

template<typename T_IN>
class device : public msg_thread<T_IN>{

public:
    using tBase = msg_thread<T_IN>;
    using t_IN_PTR = shared_ptr<T_IN>;
    device(function<bool(t_IN_PTR&&)> func_process, string name = ""):
            func_process(func_process), tBase(QUEUE_POLICY::drop/*not used*/, name){}

protected:
    virtual bool process_usr_msg(t_IN_PTR&& msg){
        return func_process(move(msg));
    };
private:
    function<bool(t_IN_PTR&&)> func_process;
};

//************************FILTER***********************
template<typename T_IN, typename T_OUT>
class filter : public msg_thread<T_IN>{
    static_assert(is_base_of<MESSAGE,T_OUT>(),"T2 shell be derived from MESSAGE class");

public:
    using tBase = msg_thread<T_IN>;
    using t_IN_PTR = shared_ptr<T_IN>;
    using t_OUT_PTR = shared_ptr<T_OUT>;

    filter(function<t_OUT_PTR(t_IN_PTR&&)> func_filter, QUEUE_POLICY pol = QUEUE_POLICY::drop, string name = ""):
            func_filter(func_filter), msg_thread<T_IN>(pol,name) {}

    void set_target(shared_ptr<msg_thread<T_OUT>> target){next = target;}

protected:
    virtual bool process_usr_msg(t_IN_PTR&& msg){
        if(next){
            return next->put(func_filter(move(msg)),tBase::pol);
        }else{
            cerr << "broken pipe detected" << endl;
            return false;
        }
    };

private:
    shared_ptr<msg_thread<T_OUT>> next;
    function<t_OUT_PTR(t_IN_PTR&&)> func_filter;
};

#endif //COMPPHYSFWK_PIPELINE_HPP
