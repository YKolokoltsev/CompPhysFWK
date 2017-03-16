//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <list>
#include <memory>
#include <set>

#include <boost/circular_buffer.hpp>


using namespace std;
using namespace boost;

const int lp = 2;
const int lq = 1;
constexpr int L = lp + lq;

class SWP_Proc{

    /*
     * the size and contents
     */
    using t_data = vector<char>;
    using t_pkt = vector<char>;
    using p_data = std::shared_ptr<t_data>;

//OUT WORLD FUNCTIONS
public:
    struct IOFuncs{
        /*
         * Client side functions (not blocking!):
         * write - passes a received data cell to the client
         * read - requests for an input packet to be sent from the client,
         * returns false if nothing to read at the moment.
         */
        function<void(const t_data&)> write;
        function<t_data ()> read;

        /*
         * Communication interface functions (not blocking!):
         * send - send any plain packet to remote process (can be another thread, process or a remote machine)
         * receive - receives a packet (if something is really wrong: return nullptr)
         * is_empty - check if a receive channel queue is empty (needed for non-destructive event applicability check)
         */
        function<void(const t_pkt& )> send;
        function<t_pkt ()> receive;
        function<bool ()> is_empty;
    };

//BASIC DATA STRUCTS
private:

    struct pkt_head{
        /*
         * I - index of a packet in infinite queue,
         * I_m =(by def.)= I mod 2*L
         */
        int I_m;

        /*
         * size of data in bytes following packet header
         */
        size_t sz_data;
    };

    struct proc_state : public IOFuncs{
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
        circular_buffer<p_data> in_pool;

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
    };

//EVENTS
private:

    /*
     * generic event interface is useful for
     *  - modification of implemented event set: we can simply include or exclude some events
     *  - inheritance of SWP_Proc: opens flexibility in different implementations of the same events
     *  - simulations: when a Proc is running within an external environment
     *  - etc...
     */
    class i_event{
    public:
        virtual bool is_valid(const proc_state& s){};
        virtual void apply(proc_state& s){};
    };

    //RECEIVE EVENT
    struct e_rcv : public i_event{

        bool is_valid(const proc_state& s){
            return !s.is_empty();
        }

        void apply(proc_state& s){
            //take one packet from the communication channel
            t_pkt pkt = s.receive();
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
            if(D < 0 || s.out_pool.at(D)) return;

            //store received data
            s.out_pool.at(D) = p_data(new t_data(std::move(m->data)));

            //detect a delivery confirmation for (D - lq + 1 - s.Dap) messages:
            //rise plain Ap (as well as DAp), eliminate delivered data from in_pool
            //and take some more data from input queue
            if(s.DAp < (D - lq + 1 ) ){
                while(s.DAp != D - lq +1){
                    //eliminate delivered data cell
                    s.in_pool.pop_front();

                    //append next data cell to input pool
                    //todo: move it to separate event?
                    s.in_pool.push_back(p_data(new t_data(s.read())));

                    s.DAp++;
                };
            }

            //move Sp (and update DAp)
            if(D == 0){
                int dSp = 0;
                while(s.out_pool.at(0)){
                    s.write(std::move(*s.out_pool.front()));
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

        bool is_valid(const proc_state& s){
            return !s.out_pool.empty();
        }

        void apply(proc_state& s){
            //get random message from in_pool
            int i = (rand() % (int)(s.in_pool.size()));
            pkt_head msg{
                    .I_m = div(s.DAp + s.Sp_m + i,2*L).rem,
                    .data = *s.in_pool.at(i)};

            t_pkt pkt(sizeof(msg));
            memcpy(pkt.data(),&msg,sizeof(msg));
            s.send(pkt);
        }
    };


public:
    SWP_Proc(const IOFuncs& f){
        (IOFuncs) state = f;
        using p_e = unique_ptr<i_event>;
        events.insert(unique_ptr<i_event>(new e_rcv));
        events.insert(unique_ptr<i_event>(new e_send));
        cout << "Num. registered events: " << events.size() << endl;
    }

private:
    proc_state state;
    set<unique_ptr<i_event>> events;
};







int main (){


    return 0;
}