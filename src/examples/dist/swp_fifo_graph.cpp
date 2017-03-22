//
// Created by morrigan on 21/03/17.
//

#include <iostream>
#include <vector>
#include <memory>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;

template<int lp, int lq>
class SWP_FIFO_Graph : public SWP_FIFO<lp,lq>{
public:
    using t_This = SWP_FIFO_Graph<lp,lq>;
    using t_Remote = SWP_FIFO_Graph<lq,lp>;
    using t_Base = SWP_FIFO<lp,lq>;
    using t_Plain = vector<char>;

private:
    const struct IOFuncs : public t_Base::IOFuncs{

        void send(const t_Plain &pkt) {}
        t_Plain receive() { return t_Plain(); }
        bool is_empty() { return true; }

        IOFuncs(){
            t_Base::IOFuncs::write = [] (const t_Plain &data) -> void {}; //write stub
            t_Base::IOFuncs::read = [] () -> t_Plain {t_Plain();}; //read stub
            t_Base::IOFuncs::receive = &receive;
            t_Base::IOFuncs::send = &send;
            t_Base::IOFuncs::is_empty = &is_empty;
        }
    } F;

public:
    SWP_FIFO_Graph(string uid) : t_Base(F, uid) {};

    set<t_This> apply_all(){
        set<t_This> clones;
        for(auto e : t_Base::events) {
            if(e->is_valid(t_Base::state)) {
                t_This clone(t_Base::uid);
                clone.state = t_Base::state;
                e->apply(clone.state);
                clones.insert(clone);
            }
        }
    }

private:
    std::shared_ptr<t_This> prev_loc; //
    std::shared_ptr<t_This> prev_rem; //case of receive msg

};

int main (int argc, char** argv){

    const int lp = 0;
    const int lq = 1;

    return 0;
}

