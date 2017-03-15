//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <list>

#include <boost/circular_buffer.hpp>
#include <boost/math/special_functions/sign.hpp>
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
const int lq = 1;
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

    for(int Sp = -100; Sp < 12345; Sp++){
        //Arbitrary I from range in Qp
        int I = Sp-L + (rand() % (2*L));

        //calc modulus
        int Sp_m = div(Sp,2*L).rem; //stored within process state
        int I_m = div(I,2*L).rem; //received by message

        //d_r - difference between modulus
        int d_r = Sp_m-I_m;

        //d_q === div(I,2*L).quot - div(Sp,2*L).quot; (difference between qoutes)
        int d_q = d_r == L ? 0 : div(d_r,L).quot;

        //D === I - Sp - difference between absolute values
        //if D < 0 => we get already received packet that is out of the Out_p array
        // -> 100% msg retransmit (no new info)
        int D = d_q*2*L-d_r;

        //check result
        if(D != (I - Sp))
        cout << D << "  vs  " << (I - Sp) << endl;
    }

    int a = 12345;
    auto res = div(a,2*L);
    cout << res.quot << " " << res.rem << "  " << a << "=" << (res.quot*2*L+res.rem) << endl;

    return 0;
}