//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <list>

#include <boost/circular_buffer.hpp>

using namespace std;
using namespace boost;

template <typename T_STATE>
class i_event{
public:
    virtual bool is_valid(const T_STATE& s) = 0;
    virtual void apply(T_STATE& s) = 0;
};

const int lp = 2;

struct msg{
    char di; //displacement from the top of out stream
};

struct proc_state{
    circular_buffer<msg> out_pool;
    list<msg> input_queue;
    int32_t wr_bitmask;
};

class e_rcv : public i_event<proc_state>{
public:
    bool is_valid(const proc_state& s){
        return !s.input_queue.empty();
    }

    void apply(proc_state& s){
        auto m = s.input_queue.front();
        s.input_queue.pop_front();
        
    }
};



int main (){
    circular_buffer<msg> buf(3);

    buf.push_back(msg{1});
    buf.push_back(msg{2});
    buf.push_back(msg{3});

    buf.rotate(buf.begin()+1);
    buf.rotate(buf.begin()+1);
    buf.rotate(buf.begin()+2);

    cout << buf.size() << endl;

    for(auto m : buf){
        cout << (int)m.di << endl;
    }

    return 0;
}