//
// Created by morrigan on 1/31/17.
//

#include <chrono>

#include "../../utils/pipeline.hpp"

using namespace std;

struct MY_MSG : public MESSAGE{
    double val;
};

//use of aliases permits type fast substitution,
//this is the case for MY_MSG struct - it can be changed
//between any piped pair (src-filter, filter-dev)
using tDevice = BaseNode<MY_MSG>;
using tSource = source<MY_MSG>;
using tFilter = filter<MY_MSG,MY_MSG>;

tSource::t_OUT_PTR src_proc(){
    tSource::t_OUT_PTR p_msg(new MY_MSG);
    p_msg->val = 1.4;
    return p_msg;
}

tFilter::t_OUT_PTR filter_proc(tFilter::t_IN_PTR&& in){
    tFilter::t_OUT_PTR out(new MY_MSG);
    out->val = in->val + 1;
    return out;
}

bool dev_proc(tDevice::t_IN_PTR&& msg){
    cout << msg->val << endl;
    return true;
}

int main(int argc, char** argv){
    //better use dynamic memory, at the moment of filter linkage
    //in any case there shell be used pointers, so if these object
    //are created in stack a broken exception can happen at distruction
    //... this is the fast solution, a todo needed
    shared_ptr<tSource> src(new tSource(src_proc), [=](tSource* p_src){p_src->stop();});
    shared_ptr<tFilter> f(new tFilter(filter_proc), [=](tFilter* p_filter){p_filter->stop();});
    shared_ptr<tDevice> dev(new tDevice(dev_proc), [=](tDevice* p_dev){p_dev->stop();});

    //specify filter chain
    src->set_target(f);
    f->set_target(dev);
    
    //start threads
    src->start();
    f->start();
    dev->start();

    //send some dummy messages to the source, each of these messages will produce a single
    //src_proc call,  this is a simplest source use case,
    //the more advanced source will need to store it's current state, so it is better
    //to overload source class and reimplement it's main loop
    for(int i = 0; i < 100; i++){
        cout << "put src " << src->put(tSource::t_IN_PTR(new MESSAGE),QUEUE_POLICY::wait) << endl;
    }

    //sometimes not all dev_proc calls actually print a text, it happens because a console is an asynchronous
    //system resource and when the process in which a text was sent is closed before the main app, its cout
    //is flushed. To che this you can add some delay before exit

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;
}