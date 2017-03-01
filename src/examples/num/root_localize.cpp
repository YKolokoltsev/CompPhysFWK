/*
 * numerov.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include "../../lib_visual/gnuplot_iostream.hpp"
#include "../../lib_num/root_localize.hpp"

template<typename T>
struct f{
    T operator() (T x, void* params = nullptr) {
        return sin(2.0*x)*exp(-0.5*x);
    }
};

template<typename T>
struct df{
    T operator() (T x, void* params = nullptr) {
        return 2.0*exp(-0.5*x)*cos(2.0*x) - 0.5*exp(-0.5*x)*sin(2.0*x);
    }
};

int main(int argc, char** argv){

    using t_I = t_interval<double>;
    f<double>  mf;
    df<double> mdf;

    t_I a{-0.3,10};
    auto locs = localize_roots<double,f,df>(a, nullptr);

    cout << "Number of intervals: " << locs.size() << endl;

    //Graphics
    using t_line = vector<pair<double, double>>;

    t_line func;
    double MAX = mf(a.lower()), MIN = mf(a.lower());
    for(double x = a.lower(); x <= a.upper(); x += 0.01){
        func.push_back(make_pair(x,mf(x)));
        MAX = max(mf(x), MAX);
        MIN = min(mf(x), MIN);
    }


    list<t_line> regions;
    for(auto x : locs){
        t_line curr;
        curr.push_back(make_pair(x.lower(),MAX));
        curr.push_back(make_pair(x.upper(),MAX));
        curr.push_back(make_pair(x.upper(),MIN));
        curr.push_back(make_pair(x.lower(),MIN));
        regions.push_back(curr);
    }


    Gnuplot gp;

    gp << "set style fill transparent solid 0.3 noborder" << endl;
    gp << "set xzeroaxis" << endl;
    gp << "plot  ";
    for(auto& c : regions) gp << " '-' with filledcurve lc rgb \"gold\" notitle, ";
    gp << " '-' with lines notitle" << endl;

    for(auto& c : regions) gp.send1d(c);
    gp.send1d(func);

    return 0;
}