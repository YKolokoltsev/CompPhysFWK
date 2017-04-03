//
// Created by morrigan on 21/03/17.
//

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/graph/graphml.hpp>

#include "helpers.hpp"

using namespace boost;
using namespace std;

int main (int argc, char** argv){

    NodeStore ghosts;
    Graph g;

    auto idx = add_vertex(g);
    ghosts.insert(node_idx(true,t_pp(new t_p("p")),nullptr,idx));
    put(vertex_name_t(), g, idx, *ghosts.get<node_idx::ByIdx>().find(idx));

    idx = add_vertex(g);
    ghosts.insert(node_idx(false,nullptr,t_pq(new t_q("q")),idx));
    put(vertex_name_t(), g, idx, *ghosts.get<node_idx::ByIdx>().find(idx));


    bool changed = true;
    while(changed) {
        changed = false;
        //Apply local events
        for (const auto &x : ghosts) {
            if(x.is_p){
                auto locals_p = x.p_p->apply_local();
                for(const auto &p: locals_p){
                    auto it_p = ghosts.get<node_idx::ByP>().find(*p);
                    if(it_p == ghosts.get<node_idx::ByP>().end()){
                        idx = add_vertex(g);
                        ghosts.insert(node_idx(true,p,nullptr,idx));
                        put(vertex_name_t(), g, idx, *ghosts.get<node_idx::ByIdx>().find(idx));
                        add_edge(x.idx,idx,g);
                        changed = true;
                    }else{
                        if(!edge(x.idx,(*it_p).idx,g).second)
                        add_edge(x.idx,(*it_p).idx,g);
                    }
                }
            }else{
                auto locals_q = x.p_q->apply_local();
                for(const auto &q: locals_q){
                    auto it_q = ghosts.get<node_idx::ByQ>().find(*q);
                    if(it_q == ghosts.get<node_idx::ByQ>().end()){
                        idx = add_vertex(g);
                        ghosts.insert(node_idx(false,nullptr,q,idx));
                        put(vertex_name_t(), g, idx, *ghosts.get<node_idx::ByIdx>().find(idx));
                        add_edge(x.idx,idx,g);
                        changed = true;
                    }else{
                        if(!edge(x.idx,(*it_q).idx,g).second)
                            add_edge(x.idx,(*it_q).idx,g);
                    }
                }
            }
        }


        //Apply receive messages
    }

    //OUTPUT STATISTICS

    // initialize dynamic_properties interface
    dynamic_properties dp;
    dp.property("name", get(vertex_name_t(), g));

    //output graph
    //write_graphml(std::cout, g, dp, true);

    auto abs_path = filesystem::complete("out.graphml");
    std::ofstream file(abs_path.string());
    write_graphml(file, g, dp, true);
    std::cout << "graph written: " << abs_path << std::endl;
    file.close();

    cout << "total ghosts: " << ghosts.size() << endl;
    cout << "total nodes: " << add_vertex(g) << endl;

    return 0;
}

