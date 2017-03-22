//
// Created by morrigan on 21/03/17.
//

#include <iostream>
#include <vector>
#include <algorithm>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;
//using namespace boost;

const int lp = 0;
const int lq = 1;

std::list<std::vector<char>> in_p;
std::list<std::vector<char>> in_q;

/*
 * A stubbed IO interface for SWP protocol,
 * here we are not interested in information transfer
 */
namespace swp_io {
    void write(const vector<char> &data) {}
    std::vector<char> read() { return std::vector<char>(); }
    template<std::list<std::vector<char>> &queue>
    void send(const vector<char> &pkt) { queue.push_back(pkt); }
    template<std::list<std::vector<char>> &queue>
    vector<char> receive() {
        vector<char> out(std::move(queue.front()));
        queue.pop_front();
        return out;
    }
    template<std::list<std::vector<char>> &queue>
    bool is_empty() { return queue.empty(); }
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
        cout << t_Base::uid << *this << " ";
        if(applicable.empty()){cout << "no applicapable events" << endl; return;}
        int i = (rand() % (int)(applicable.size()));
        cout << applicable.size() << " ";
        auto it = applicable.begin(); while(i != 0){i--; it++;}
        cout << "applying: " << (*it)->get_uid() << endl;
        (*it)->apply(t_Base::state);
    }
};

int main (int argc, char** argv){
    typedef SWP_FIFO_Rand<lp,lq> t_SWPp;
    typedef SWP_FIFO_Rand<lq,lp> t_SWPq;

    t_SWPp::IOFuncs Fp;
    Fp.read = &swp_io::read;
    Fp.write = &swp_io::write;
    Fp.send = &swp_io::send<in_q>;
    Fp.receive = &swp_io::receive<in_p>;
    Fp.is_empty = &swp_io::is_empty<in_p>;
    t_SWPp p(Fp, "p");

    t_SWPq::IOFuncs Fq;
    Fq.read = &swp_io::read;
    Fq.write = &swp_io::write;
    Fq.send = &swp_io::send<in_p>;
    Fq.receive = &swp_io::receive<in_q>;
    Fq.is_empty = &swp_io::is_empty<in_q>;
    t_SWPq q(Fq, "q");

    for(int i = 0; i < 1000; i++) {
        p.apply_random_event();
        q.apply_random_event();
    }

    return 0;
}

