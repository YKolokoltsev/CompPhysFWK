//
// Created by morrigan on 1/04/17.
//

#ifndef COMPPHYSFWK_SWP_FIFO_GHOST_H
#define COMPPHYSFWK_SWP_FIFO_GHOST_H

#include <iostream>
#include <vector>
#include <memory>
#include <bitset>
#include <type_traits>

#include "../../../lib_dist/swp_fifo.hpp"

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
    const ExIOFuncs* rcv_from{nullptr};
};

template<int lp, int lq>
class SWP_FIFO_Ghost : public SWP_FIFO<lp,lq,ExIOFuncs>{

public:

    using t_Base = SWP_FIFO<lp, lq, ExIOFuncs>;
    using t_Local = SWP_FIFO_Ghost<lp, lq>;
    using t_Remote = SWP_FIFO_Ghost<lq, lp>;

    using p_e = std::shared_ptr<typename t_Base::i_event>;

    SWP_FIFO_Ghost(string uid) : t_Base(ExIOFuncs(), uid) {
        local_events.push_back(p_e(new typename t_Base::e_read));
        for(int idx = 0; idx < t_Base::L; idx++)
            local_events.push_back(p_e(new typename t_Base::e_send(idx)));
    }

    bool has_sent() const{
        return t_Base::state.f.sent_pkt != nullptr;
    }

    list<std::shared_ptr<t_Local>> apply_local() const{
        list<std::shared_ptr<t_Local>> clones;
        for(auto e : local_events) {
            auto c = clone();
            if(e->is_valid(t_Base::state)) {
                e->apply(c->state);
                c->ev_prev = e->get_uid();
                clones.push_back(c);
            }
        }
        return clones;
    }

    std::shared_ptr<t_Local> apply_receive(const std::shared_ptr<t_Remote>& p_neigh) const{
        if(p_neigh->state.f.sent_pkt == nullptr)
            throw runtime_error("receive try on not sent event");
        auto c = clone();
        c->state.f.rcv_from = &p_neigh->state.f;
        typename t_Base::e_rcv rcv;
        rcv.apply(c->state);
        return c;
    }

    bool operator< (const t_Local& a) const{

        if(a.uid != this->uid){
            return a.uid < this->uid;
        }else if(a.state.Sp_m != this->state.Sp_m){
            //cout << "Sp_m" << endl;
            return a.state.Sp_m < this->state.Sp_m;
        }else if(a.state.DAp != this->state.DAp){
            //cout << "DA_p" << endl;
            return a.state.DAp < this->state.DAp;
        }else if(a.state.in_pool.size() != this->state.in_pool.size()){
            //cout << "in_pool" << endl;
            return a.state.in_pool.size() < this->state.in_pool.size();
        }else if(bits(a) != bits(*this)){
            //cout << "bits" << endl;
            return bits(a) < bits(*this);
        }else if(a.state.f.sent_pkt != this->state.f.sent_pkt){
            //cout << "sent_pkt" << endl;
            if(a.state.f.sent_pkt == nullptr) return false;
            if(this->state.f.sent_pkt == nullptr) return true;

            //todo: remove explicit?
            using pkt_head = typename t_Base::pkt_head;
            pkt_head* h_a = static_cast<pkt_head*>((void*)a.state.f.sent_pkt->data());
            pkt_head* h_this = static_cast<pkt_head*>((void*)this->state.f.sent_pkt->data());

            return h_a->I_m < h_this->I_m;
            //return *a.state.f.sent_pkt < *this->state.f.sent_pkt;
        }else if(a.state.f.rcv_from != this->state.f.rcv_from){
            //cout << "rcv_from" << endl;
            return a.state.f.rcv_from < this->state.f.rcv_from;
        }else{
            //cout << "equal" << endl;
            return 0;
        }
    }

    friend std::ostream& operator<< (std::ostream& stream, const t_Local& node) {
        using t_Base = typename SWP_FIFO_Ghost<lp,lq>::t_Base;
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

    std::shared_ptr<t_Local> clone() const{
        std::shared_ptr<t_Local> c(new t_Local(this->uid));
        c->local_prev = this;
        c->state = this->state;
        c->state.f.sent_pkt = nullptr;
        c->state.f.rcv_from = nullptr;
        return c;
    }

    static int bits(const t_Local& node){
        static_assert(t_Base::L < sizeof(int)*8-1, "change this algorithm");
        int bits = 0;
        for(auto& x : node.state.out_pool){bits |= (int)(x != nullptr); bits <<= 1;}
        return bits;
    }

private:
    std::list<p_e> local_events; //Todo: -> SET
    const t_Local* local_prev{nullptr};
    string ev_prev{"none"};
};

#endif //COMPPHYSFWK_SWP_FIFO_GHOST_H
