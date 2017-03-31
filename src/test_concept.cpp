/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */
#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <fstream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>

using namespace boost;

/*
int main(int,char*[])
{
    // create a typedef for the Graph type
    typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;

    // Make convenient labels for the vertices
    enum { A, B, C, D, E, N };
    const int num_vertices = N;
    const char* name = "ABCDE";

    // writing out the edges in the graph
    typedef std::pair<int, int> Edge;
    Edge edge_array[] =
            { Edge(A,B), Edge(A,D), Edge(C,A), Edge(D,C),
              Edge(C,E), Edge(B,D), Edge(D,E) };
    const int num_edges = sizeof(edge_array)/sizeof(edge_array[0]);

    // declare a graph object
    Graph g(num_vertices);

    // add the edges to the graph object
    for (int i = 0; i < num_edges; ++i)
        add_edge(edge_array[i].first, edge_array[i].second, g);

    //show graph with GraphViz
    std::ofstream file("/home/data/tmp/out.gv");
    //write_graphml(std::cout, g, make_label_writer(name));
    write_graphviz(std::cout, g, make_label_writer(name));
    write_graphviz(file, g, make_label_writer(name));
    file.close();

    return 0;
}*/

enum files_e { dax_h, yow_h, boz_h, zow_h, foo_cpp,
    foo_o, bar_cpp, bar_o, libfoobar_a,
    zig_cpp, zig_o, zag_cpp, zag_o,
    libzigzag_a, killerapp, N };
const char* name[] = { "dax.h", "yow.h", "boz.h", "zow.h", "foo.cpp",
                       "foo.o", "bar.cpp", "bar.o", "libfoobar.a",
                       "zig.cpp", "zig.o", "zag.cpp", "zag.o",
                       "libzigzag.a", "killerapp" };

#include <utility>
#include <string>

int main(int,char*[])
{
    typedef std::pair<int,int> Edge;
    Edge used_by[] = {
            Edge(dax_h, foo_cpp), Edge(dax_h, bar_cpp), Edge(dax_h, yow_h),
            Edge(yow_h, bar_cpp), Edge(yow_h, zag_cpp),
            Edge(boz_h, bar_cpp), Edge(boz_h, zig_cpp), Edge(boz_h, zag_cpp),
            Edge(zow_h, foo_cpp),
            Edge(foo_cpp, foo_o),
            Edge(foo_o, libfoobar_a),
            Edge(bar_cpp, bar_o),
            Edge(bar_o, libfoobar_a),
            Edge(libfoobar_a, libzigzag_a),
            Edge(zig_cpp, zig_o),
            Edge(zig_o, libzigzag_a),
            Edge(zag_cpp, zag_o),
            Edge(zag_o, libzigzag_a),
            Edge(libzigzag_a, killerapp)
    };

    const int nedges = sizeof(used_by)/sizeof(Edge);

    typedef adjacency_list< vecS, vecS, directedS,
            property< vertex_color_t, std::string >,
            property< edge_weight_t, int >
    > Graph;
    Graph g(used_by, used_by + nedges, N);

    graph_traits<Graph>::vertex_iterator v, v_end;
    for (tie(v,v_end) = vertices(g); v != v_end; ++v)
        put(vertex_color_t(), g, *v, name[*v]);

    graph_traits<Graph>::edge_iterator e, e_end;
    for (tie(e,e_end) = edges(g); e != e_end; ++e)
        put(edge_weight_t(), g, *e, 3);

    dynamic_properties dp;
    dp.property("name", get(vertex_color_t(), g));
    dp.property("weight", get(edge_weight_t(), g));


    //write to file
    std::ofstream file("/home/data/tmp/out.graphml");
    write_graphml(file, g, dp, true);
    write_graphml(std::cout, g, dp, true);
    file.close();
}