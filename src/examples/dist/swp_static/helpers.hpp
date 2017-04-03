//
// Created by morrigan on 2/04/17.
//

#ifndef COMPPHYSFWK_HELPERS_H
#define COMPPHYSFWK_HELPERS_H

#include <set>
#include <algorithm>
#include <memory>
#include <map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <boost/filesystem.hpp>

#include "SWP_FIFO_Ghost.hpp"

using namespace std;

const int lp = 5;
const int lq = 3;

typedef SWP_FIFO_Ghost<lp,lq> t_p;
typedef SWP_FIFO_Ghost<lq,lp> t_q;

using t_pp = std::shared_ptr<t_p>;
using t_pq = std::shared_ptr<t_q>;

struct node_idx{
    node_idx(){};
    node_idx(bool is_p, const t_pp p_p, const t_pq p_q,unsigned long int idx):
            is_p{is_p},p_p{p_p},p_q{p_q},idx{idx}{};
    bool is_p{false};
    t_pp p_p{nullptr};
    t_pq p_q{nullptr};
    unsigned long int idx{0};

    bool has_sent(){
        if(is_p && p_p) return p_p->has_sent();
        if(!is_p && p_q) return p_q->has_sent();
        cerr << "inspecting has_sent on null" << endl;
        return false;
    }

    t_p deref_p() const {
        if(p_p == nullptr) return t_p("np");
        return *p_p;
    }

    t_q deref_q() const {
        if(p_q == nullptr) return t_q("nq");
        return *p_q;
    }

    struct ByP{}; struct ByQ{}; struct ByIdx {};

    friend std::ostream& operator<< (std::ostream& stream, const node_idx& node){
        if(node.is_p && node.p_p) stream << *node.p_p;
        if(!node.is_p && node.p_q) stream << *node.p_q;
        return stream;
    }

    friend std::istream& operator>> (std::istream& stream, node_idx& node){
        cerr << ">> stub" << endl;
        string name;
        stream >> name;
        return stream;
    }
};

using namespace boost::multi_index;
typedef boost::multi_index_container<node_idx,
        indexed_by<
                ordered_unique<
                        tag<node_idx::ByIdx>,
                        member<node_idx,unsigned long int,&node_idx::idx>,
                        less<unsigned long int>>,
                ordered_non_unique<
                        tag<node_idx::ByP>,
                        const_mem_fun<node_idx,t_p,&node_idx::deref_p>>,
                ordered_non_unique<
                        tag<node_idx::ByQ>,
                        const_mem_fun<node_idx,t_q,&node_idx::deref_q>>>,
        allocator<node_idx>
> NodeStore;

typedef adjacency_list<vecS, vecS, bidirectionalS,
        property< vertex_name_t, node_idx >> Graph;

#endif //COMPPHYSFWK_HELPERS_H
