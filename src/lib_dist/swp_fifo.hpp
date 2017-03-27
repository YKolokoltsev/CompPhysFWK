//
// Created by morrigan on 16/03/17.
//

#ifndef COMPPHYSFWK_SWP_FIFO_H
#define COMPPHYSFWK_SWP_FIFO_H

#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include <boost/circular_buffer.hpp>


using namespace std;
using namespace boost;

struct IOFuncs {
    /*
         * Client side functions (not blocking!):
         * write - passes a received data cell to the client
         * read - requests for an input packet to be sent from the client,
         * returns false if nothing to read at the moment.
         */
    virtual void write(const std::vector<char>& w){};
    virtual std::vector<char> read(){return std::vector<char>();};

    /*
     * Communication interface functions (not blocking!):
     * send - send any plain packet to remote process (can be another thread, process or a remote machine)
     * receive - receives a packet (if something is really wrong: return nullptr)
     * is_empty - check if a receive channel queue is empty (needed for non-destructive event applicability check)
     */
    virtual void send(const std::vector<char>& ){};
    virtual std::vector<char> receive(){return std::vector<char>();};
    virtual bool is_empty() const {return true;};
};


/*
 * lp - local process ack retard
 * lq - remote process ack retard
 */
template<int lp, int lq, typename T_IO>
class SWP_FIFO{
protected:
    using p_data = std::shared_ptr<vector<char>>;
    static constexpr int L = lp + lq;

    static_assert(std::is_base_of<IOFuncs,T_IO>::value,"Unrecognized IO interface");
    static_assert(L != 0,"Zero length window");

//BASIC DATA STRUCTS
protected:

    struct pkt_head{
        /*
         * I - index of a packet in infinite queue,
         * I_m =(by def.)= I mod 2*L
         */
        int I_m;

        /*
         * size of data in bytes following packet header of sizeof(pkt_head) bytes
         */
        size_t sz_data;
    };

    struct proc_state {
        proc_state(const T_IO& f): f{f} {};

        /*
         * out_pool - is an ordered set of received data cells where zero
         * index coincide with plain Sp index in any accessible state. out_pool
         * always contains a nullptr at zero index
         *
         * in_pool - is an ordered set of outgoing messages,  its zero
         * index coincide with plain Ap
         *
         * Note: circular buffer permits efficient indexed access .at(...)
         * as well as push/pop functionality, buffer rotation is not used
         * in this implementation
         */
        circular_buffer<p_data> out_pool{L,nullptr};
        circular_buffer<p_data> in_pool{L};

        /*
         * Sp - the lowest numbered word that process 'p' still expects from 'q'
         * Sp_m - is the modulus of the Sp after division by 2*L
         */
        int Sp_m = 0;

        /*
         * Ap - the lowest numbered word for which an acknowledgement has not yet been received
         * DAp = Ap - Sp
         */
        int DAp = 0;

        /*
         * External interface
         */
        T_IO f;
    };

//EVENTS
protected:

    /*
     * generic event interface is useful for
     *  - modification of implemented event set: we can simply include or exclude some events
     *  - inheritance of SWP_Proc: opens flexibility in different implementations of the same events
     *  - simulations: when a Proc is running within an external environment
     *  - etc...
     */
    class i_event{
    public:
        virtual bool is_valid(const proc_state& s) = 0;
        virtual void apply(proc_state& s) = 0;
        string get_uid(){return uid;}
    protected:
        i_event(string uid): uid(uid){};
    private:
        string uid;
    };

    //RECEIVE EVENT
    struct e_rcv : public i_event{
        e_rcv():i_event("e_rcv"){}

        bool is_valid(const proc_state& s){
            return !s.f.is_empty();
        }

        void apply(proc_state& s){
            //take one packet from the communication channel
            vector<char> pkt = s.f.receive();
            //this can happen only if the external receive channel was modified after is_valid() call
            if(pkt.empty()){ cerr << "empty packet"; return; }

            pkt_head* m = static_cast<pkt_head*>((void*)pkt.data());

            //d_r - difference between modulus (intermediate variable)
            int d_r = s.Sp_m - m->I_m;

            //d_q =(by def.)= div(I,2*L).quot - div(Sp,2*L).quot; (difference between qoutes)
            //by required channel FIFO property d_q in {-1,0,1}
            int d_q = d_r == L ? 0 : div(d_r,L).quot;

            //D =(by def.)= (I - Sp) - difference between absolute values
            //possibility to calculate D of plain indexes permits
            //to have a finite number of accessible states, this is pure math.
            int D = d_q*2*L-d_r;

            //it is a retransmit, no new information received
            if(D < 0 || s.out_pool.at(D) != nullptr) { return; }

            //store received data
            s.out_pool.at(D) = p_data(new vector<char>(m->sz_data));
            memcpy(s.out_pool.at(D)->data(),pkt.data()+(pkt.size()-m->sz_data),m->sz_data);

            //detect a delivery confirmation for (D - lq + 1 - s.Dap) messages:
            //rise plain Ap (as well as DAp), eliminate delivered data from in_pool
            //---and take some more data from input queue
            if(s.DAp < (D - lq + 1) ){
                while(s.DAp != D - lq + 1){
                    //eliminate delivered data cell
                    s.in_pool.pop_front();
                    s.DAp++;
                };
            }

            //move Sp (and update DAp)
            if(D == 0){
                int dSp = 0;
                while(s.out_pool.at(0)){
                    s.f.write(std::move(*s.out_pool.front()));
                    s.out_pool.pop_front();
                    s.out_pool.push_back(nullptr);
                    dSp++;
                }
                s.Sp_m += dSp;
                s.DAp -= dSp;
            }

            if(s.Sp_m >= 2*L) s.Sp_m = div(s.Sp_m,2*L).rem;
        }
    };

    //SEND EVENT
    struct e_send : public i_event{
        e_send(int idx): i_event("e_send_" + to_string(idx)), idx{idx}{
            assert(idx >= 0 && idx < L);
        }

        bool is_valid(const proc_state& s){
            return idx < s.in_pool.size();
        }

        void apply(proc_state& s){
            size_t sz_data = s.in_pool.at(idx)->size();
            pkt_head msg{
                    .I_m = s.DAp + s.Sp_m + idx,
                    .sz_data = sz_data};
            if(msg.I_m < 0) msg.I_m += 2*L;
            if(msg.I_m >= 2*L) msg.I_m -= 2*L;

            vector<char> pkt(sizeof(msg)+sz_data);
            memcpy(pkt.data(),&msg,sizeof(msg));
            memcpy(pkt.data()+sizeof(msg),s.in_pool.at(idx)->data(),sz_data);
            s.f.send(pkt);
        }
    private:
        const int idx;
    };

    //READ EVENT
    struct e_read : public i_event{
        e_read():i_event("e_read"){}

        bool is_valid(const proc_state& s){
            return (s.DAp + (int)s.in_pool.size() < lp);
        }

        void apply(proc_state& s){
            while(s.DAp + (int)s.in_pool.size() < lp)
                s.in_pool.push_back(p_data(new vector<char>(s.f.read())));
        }
    };


public:
    SWP_FIFO(const T_IO& f, string uid): state{f}, uid{uid} {}

    friend std::ostream& operator<< (std::ostream& stream, const SWP_FIFO<lp,lq,T_IO>& node){
        stream << node.uid << ":[.in_shortage=" << (lp - node.state.DAp - (int)node.state.in_pool.size()) << ",";
        stream << ".DAp=" << node.state.DAp << ",";
        stream << ".Sp_m=" << node.state.Sp_m << "]";
        return stream;
    }

protected:
    /*
     * Internal process state as defined in "proc_state" struct excluding
     * interface functions to the out world.
     */
    proc_state state;

    /*
     * unique process ID
     */
    string uid;
};

#endif //COMPPHYSFWK_SWP_FIFO_H
