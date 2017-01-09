//
// Created by morrigan on 6/01/17.
//

#ifndef COMPPHYSFWK_GNUPLOT_IOSTREAM_HPP
#define COMPPHYSFWK_GNUPLOT_IOSTREAM_HPP

#include <vector>
#include <tuple>
#include <list>
#include <gnuplot-iostream.h>

using namespace std;

using plot = vector<tuple<double,double>>;
using t_list_plots = vector<plot>;

void show(t_list_plots data){
    Gnuplot gp;
    bool is_first = true;
    for(auto line : data){
        if(is_first){ gp << "plot ";} else{gp << ", "; };
        is_first = false;
        gp << " '-' with lines";
    }
    gp << ";" << endl;

    for(auto line : data){
        gp.send1d(line);
    }
}

#endif //COMPPHYSFWK_GNUPLOT_IOSTREAM_HPP
