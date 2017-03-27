//
// Created by morrigan on 21/03/17.
//

#include <iostream>
#include <vector>
#include <memory>
#include <bitset>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;

/*
 * In the causal event graph each node is friezed SWP_FIFO ghost process;
 * Here "send"/"receive" interface is quite specific: sent message is stored
 * inside the sender ghost process. Received message points onto the corresponding ghost
 * with sent event.
 */
struct ExIOFuncs : public IOFuncs{
    typedef std::vector<char> t_pkt;

    virtual void send(const t_pkt& w){ sent_pkt.reset(new t_pkt{w});};
    virtual t_pkt receive(){return *rcv_from->sent_pkt;};
    virtual bool is_empty() const { return rcv_from == nullptr; };

    std::shared_ptr<t_pkt> sent_pkt{nullptr};
    ExIOFuncs* rcv_from{nullptr};
};

template<int lp, int lq>
class SWP_FIFO_Ghost : public SWP_FIFO<lp,lq,ExIOFuncs>{
public:
    using t_Base = SWP_FIFO<lp, lq, ExIOFuncs>;
    using t_Local = SWP_FIFO_Ghost<lp, lq>;
    using t_Remote = SWP_FIFO_Ghost<lq, lp>;
    using p_e = std::shared_ptr<typename t_Base::i_event>;

public:
    bool has_sent(){return state.f.sent_pkt != nullptr;}

    SWP_FIFO_Ghost(string uid) : t_Base(ExIOFuncs(), uid) {
        local_events.push_back(p_e(new typename t_Base::e_read));
        for(int idx = 0; idx < t_Base::L; idx++)
            local_events.push_back(p_e(new typename t_Base::e_send(idx)));
    };

    set<t_Local> apply_local() const {
        set<t_Local> clones;
        for(auto e : local_events) {
            auto c = clone();
            if(e->is_valid(t_Base::state)) {
                e->apply(c.state);
                c.ev_prev = e->get_uid();
                clones.insert(c);
            }
        }
        return clones;
    }

    t_Local apply_receive(const t_Remote& neigh){
        if(neigh.state.f.sent_pkt == nullptr)
            throw runtime_error("receive try on not sent event");
        auto c = clone();
        c.state.f.rcv_from = &neigh.state.f;
        t_Base::e_rcv().apply(c.state);
        return c;
    }

    int operator< (const t_Local& a) const {
        if(a.state.Sp_m != this->state.Sp_m){
            return a.state.Sp_m < this->state.Sp_m;
        }else if(a.state.DAp != this->state.DAp){
            return a.state.DAp < this->state.DAp;
        }else if(a.state.in_pool.size() != this->state.in_pool.size()){
            return a.state.in_pool.size() < this->state.in_pool.size();
        }else if(bits(a) != bits(*this)){
            return bits(a) < bits(*this);
        }else if(a.state.f.sent_pkt != this->state.f.sent_pkt){
            return a.state.f.sent_pkt < this->state.f.sent_pkt;
        }else if(a.state.f.rcv_from != this->state.f.rcv_from){
            return a.state.f.rcv_from < this->state.f.rcv_from;
        }else{
            return false;
        }
    }

    friend std::ostream& operator<< (std::ostream& stream, const t_Local& node){
        stream << node.ev_prev << "(";
        if(node.local_prev){
            stream << (t_Base)*node.local_prev;
            stream << "|" << (node.local_prev->state.f.rcv_from ? "r" : "-");
            stream << (node.local_prev->state.f.sent_pkt ? "w" : "-") << "|";
            stream << bitset<t_Base::L+1>(bits(*node.local_prev)) << "|";
        }else{
            stream << "[init]--";
        }
        stream << ") -> " << (t_Base)node;
        stream << "|" << (node.state.f.rcv_from ? "r" : "-");
        stream << (node.state.f.sent_pkt ? "w" : "-") << "|";
        stream << bitset<t_Base::L+1>(bits(node)) << "|";
        return stream;
    }

private:

    t_Local clone() const {
        t_Local c(this->uid);
        c.local_prev = this;
        c.state = this->state;
        c.state.f.sent_pkt = nullptr;
        c.state.f.rcv_from = nullptr;
        return c;
    }

    static int bits(const t_Local& node){
        static_assert(t_Base::L < sizeof(int)*8-1, "change this algorithm");
        int bits = 0;
        for(auto& x : node.state.out_pool){bits |= (int)(x != nullptr); bits <<= 1;}
        return bits;
    }

    std::list<p_e> local_events; //Todo: -> SET
    const t_Local* local_prev{nullptr};
    string ev_prev{"none"};
};

int main (int argc, char** argv){

    const int lp = 2;
    const int lq = 1;

    SWP_FIFO_Ghost<lp,lq> p("p");
    SWP_FIFO_Ghost<lq,lp> q("q");

    auto p_childs = p.apply_local();
    for(auto& c : p_childs){
        cout << c << endl;
        auto gaga = c.apply_local();
        for(auto& d : gaga){
            cout << d << endl;
        }
    }

    return 0;
}

