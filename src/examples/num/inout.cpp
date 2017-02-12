/*
 * inout.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 *
 *  If we integrate a wave equation from x0 to x1, and than
 *  taking the result of this solution at x1 integrate it
 *  backwards from x1 to x0, the resulting function shell not
 *  coincide with the starting position.
 *
 *  This example will give us a taste of how strong is this effect and also
 *  is a good test-case on boundary points.
 *
 *  ... well, the Numerov algorithm with doubles is fkn stable
 */

#include <iostream>
#include <utility>
#include <complex>

#include "../../lib_num/numerov.hpp"
#include "../../lib_visual/gnuplot_iostream.hpp"

using namespace std;

//just a constatnt wavenumber

//todo: comlpex!!!
using T = double;
T k(const T){ return 5.0; }

int main(int argc, char** argv){

    //computation parameters
    const T x0{0}, x1{3};
    const size_t N = 30;
    array<T,2> y{T{0},T{3}};

    //Numerov algorithm initialization
    const T h = (x1 - x0)/(double)N;

    //Forward integration
    Numerov<T,k,nullptr,DIR::fwd>::t_numerov num_fwd(h,x0+h);

    vector<pair<T,T>> fwd(N);
    for(size_t i = 0; i < N; i++) {
        fwd.at(i) = make_pair(abs(num_fwd.get_x()), abs(y[1]));
        num_fwd.eval_next(y);
    }

    //Backward integration
    Numerov<T,k,nullptr,DIR::bwd>::t_numerov num_bwd(h,num_fwd.get_x()-h);

    vector<pair<T,T>> bwd(N);
    for(size_t i = 0; i < N; i++) {
        bwd.at(i) = make_pair(abs(num_bwd.get_x()), abs(y[0]));
        num_bwd.eval_next(y);
    }

    //Plotting results
    Gnuplot gp;
    gp << "plot '-' with lines title 'forward', '-' with lines title 'backward'\n";
    gp.send1d(fwd);
    gp.send1d(bwd);

    return 0;
}
