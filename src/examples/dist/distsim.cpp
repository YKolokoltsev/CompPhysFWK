//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <list>

#include <boost/circular_buffer.hpp>
#include <stdlib.h>

using namespace std;
using namespace boost;

template <typename T_STATE>
class i_event{
public:
    virtual bool is_valid(const T_STATE& s) = 0;
    virtual void apply(T_STATE& s) = 0;
};

const int lp = 2;
const int lq = 2;
constexpr int L = lp + lq;

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

    for(int Sp = 10; Sp < 100; Sp++){
        //int I = rand[Sp-L, Sp+L)....
    }

    int a = 12345;
    auto res = div(a,2*L);
    cout << res.quot << " " << res.rem << "  " << a << "=" << (res.quot*2*L+res.rem) << endl;

    return 0;
}