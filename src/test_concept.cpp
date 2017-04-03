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
#include <boost/filesystem.hpp>

using namespace boost;
struct node_props{
    std::string name;

    friend std::ostream& operator<< (std::ostream& stream, const node_props& node){
        stream << node.name;
        return stream;
    }

    friend std::istream& operator>> (std::istream& stream, node_props& node){
        stream >> node.name;
        return stream;
    }
};

int main(int,char*[])
{
    // create a typedef for the Graph type
    typedef adjacency_list<vecS, vecS, bidirectionalS,
            property< vertex_name_t, node_props >> Graph;

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

    // fill vertex properties
    graph_traits<Graph>::vertex_iterator v, v_end;
    for (tie(v,v_end) = vertices(g); v != v_end; ++v)
        put(vertex_name_t(), g, *v, node_props{std::string() + name[*v]});

    // initialize dynamic_properties interface
    dynamic_properties dp;
    dp.property("name", get(vertex_name_t(), g));

    //output graph
    write_graphml(std::cout, g, dp, true);

    auto abs_path = filesystem::complete("out.graphml");
    std::ofstream file(abs_path.string());
    write_graphml(file, g, dp, true);
    std::cout << abs_path << std::endl;
    file.close();

    return 0;
}