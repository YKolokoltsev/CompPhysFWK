/*
 * test_concept.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <type_traits>
#include <iostream>
#include <algorithm>

#include "../../lib_visual/gnuplot.hpp"
#include "../../lib_dsp/sources.hpp"
#include "../../lib_dsp/filters.hpp"

using namespace std;

int main(){
    Plotter plotter("set term x11 enhanced font 'arial,15' persist");

    double sigma = 0.1;
    double RC = 5;
    double len = 5000;

    auto s0 = gaussian_pulse(0.01,10,sigma);
    auto rc = rc_filter(s0.get_dt(),len*s0.get_dt(),RC);
    auto s1 = s0.convolve(rc);

    auto irc = inverse_rc_filter(s0.get_dt(),len*s0.get_dt(),RC);
    auto s2 = s1.convolve(irc);
    s2.normalize();

    plotter.append(s0.get(), "with lines title 's0'");
    plotter.append(s1.get(), "with lines title 'RC'");
    plotter.append(s2.get(), "with lines title 'iRC'");
    //plotter.append(rc.get(), "with lines title 'RC filter'");
    //plotter.append(irc.get(), "with lines title 'IRC filter'");
    plotter.show();

    cout << s0.norm() << " " << s1.norm() << endl;


    return 0;
}
