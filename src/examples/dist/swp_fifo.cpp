//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <memory>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;

const int lp = 0;
const int lq = 1;

/*
 * Both "p" and "q" nodes has the same send/receive interface,
 * so first we define this one
 */
struct ExIOFuncs : public IOFuncs{
    virtual bool is_empty() const { return in->empty(); };
    virtual void send(const vector<char>& w){ out->push_back(w); };
    virtual vector<char> receive(){
        vector<char> w(std::move(in->front()));
        in->pop_front();
        return w;
    };

    using t_queue = std::shared_ptr<std::list<std::vector<char>>>;
    t_queue in{new t_queue::element_type()};
    t_queue out{nullptr};
};

/*
 * The "p" node just prints in console what it receives from q,
 * all the others function leaved to defaults
 */
struct ExIOFuncs_p : public ExIOFuncs{
    virtual void write(const vector<char>& w) {
        cout << string(w.begin(),w.end()) << endl;
    };
};

/*
 * The "q" node on each read creates a printable text string to be
 * sent to "p"
 */
struct ExIOFuncs_q : public ExIOFuncs{
    virtual vector<char> read(){
        vector<char> data(100);
        for(int i = 0; i < 100; i++){
            char ch = (char)(rand() % 255);
            if(isprint(ch)) data.push_back(ch);
        }
        return data;
    };
};


/*
 * This class applies a random event from all accessible
 * deterministic events. This is the worst policy for SWP protocol,
 * so we can observe a really poor efficiency.
 */
template<int lp, int lq, typename T_IO>
class SWP_FIFO_Rand : public SWP_FIFO<lp,lq,T_IO>{
public:
    using t_Base = SWP_FIFO<lp,lq,T_IO>;
    using i_event = typename t_Base::i_event;
    SWP_FIFO_Rand(const T_IO& f, string uid) : t_Base(f, uid) {
        using p_e = std::shared_ptr<i_event>;
        events.insert(p_e(new typename t_Base::e_rcv));
        events.insert(p_e(new typename t_Base::e_read));
        for(int idx = 0; idx < t_Base::L; idx++)
            events.insert(p_e(new typename t_Base::e_send(idx)));
    };

    void apply_random_event(){
        std::list<i_event*> applicable;
        for(auto e : events) {
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

private:
    /*
     * A set of all deterministic events. These are not atomic events
     * in a sense of discrete event tuples {pid, s_in, m_r, m_s, s_out},
     * each of these events can produce a concrete tuple if applied to a concrete
     * internal state + process input queue. A set of all possible atomic events is
     * much more rich than this set.
     */
    set<std::shared_ptr<i_event>> events;
};

int main (){
    ExIOFuncs_p fp;
    ExIOFuncs_q fq;

    fp.out = fq.in;
    fq.out = fp.in;

    SWP_FIFO_Rand<lp,lq, ExIOFuncs_p> p(fp,"p");
    SWP_FIFO_Rand<lq,lp, ExIOFuncs_q> q(fq,"q");

    for(int i = 0; i < 1000; i++) {
        p.apply_random_event();
        q.apply_random_event();
    }

    cout << "\n\nThis protocol is extremely sensitive to lp, lq\n";
    cout << "and probabilities of the accessible events.\n"
            "Try to play with lp, lq and apply_random_event() code.\n\n";

    return 0;
}