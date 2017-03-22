//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <vector>
#include <algorithm>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;
using namespace boost;

const int lp = 0;
const int lq = 1;

std::list<std::vector<char>> in_p;
std::list<std::vector<char>> in_q;

void write_p(const vector<char>& data){
    cout << string(data.begin(),data.end()) << endl;
}

void write_q(const vector<char>& data){
    return;
}

vector<char> read_p(){
    vector<char> data;
    return data;
}

vector<char> read_q(){
    vector<char> data(100);
    for(int i = 0; i < 100; i++){
        char ch = (char)(rand() % 255);
        if(isprint(ch)) data.push_back(ch);
    }
    return data;
}

void send_p(const vector<char>& pkt){
    in_q.push_back(pkt);
}

void send_q(const vector<char>& pkt){
    in_p.push_back(pkt);
}

vector<char> receive_p(){
    vector<char> out(std::move(in_p.front()));
    in_p.pop_front();
    return out;
}

vector<char> receive_q(){
    vector<char> out(std::move(in_q.front()));
    in_q.pop_front();
    return out;
}

bool is_empty_p(){
    return in_p.empty();
}

bool is_empty_q(){
    return in_q.empty();
}

template<int lp, int lq>
class SWP_FIFO_Rand : public SWP_FIFO<lp,lq>{
public:
    using t_Base = SWP_FIFO<lp,lq>;
    using IOFuncs = typename t_Base::IOFuncs;
    using i_event = typename t_Base::i_event;
    SWP_FIFO_Rand(const IOFuncs& f, string uid) : t_Base(f, uid) {};
    void apply_random_event(){
        std::list<i_event*> applicable;
        for(auto e : t_Base::events) {
            if(e->is_valid(t_Base::state)) {
                applicable.push_back(e.get());
            }
        }
        //cout << t_Base::uid << *this << " ";
        if(applicable.empty()){cout << "no applicapable events" << endl; return;}
        int i = (rand() % (int)(applicable.size()));
        //cout << applicable.size() << " ";
        auto it = applicable.begin(); while(i != 0){i--; it++;}
        //cout << "applying: " << (*it)->get_uid() << endl;
        (*it)->apply(t_Base::state);
    }
};

int main (){
    typedef SWP_FIFO_Rand<lp,lq> t_SWPp;
    typedef SWP_FIFO_Rand<lq,lp> t_SWPq;

    t_SWPp::IOFuncs Fp;
    Fp.read = &read_p;
    Fp.write = &write_p;
    Fp.send = &send_p;
    Fp.receive = &receive_p;
    Fp.is_empty = &is_empty_p;
    t_SWPp p(Fp, "p");

    t_SWPq::IOFuncs Fq;
    Fq.read = &read_q;
    Fq.write = &write_q;
    Fq.send = &send_q;
    Fq.receive = &receive_q;
    Fq.is_empty = &is_empty_q;
    t_SWPq q(Fq, "q");

    for(int i = 0; i < 1000; i++) {
        p.apply_random_event();
        q.apply_random_event();
    }

    cout << "\n\nThis protocol is extremely sensitive to lp, lq\n";
    cout << "and probabilities of the accessible events.\n"
            "Try to play with lp, lq and apply_random_event() code.\n\n";

    return 0;
}