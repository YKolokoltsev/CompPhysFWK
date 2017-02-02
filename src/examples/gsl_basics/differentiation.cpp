/*
 * Created by Dr. Yevgeniy Kolokoltsev on 6 Jan 17.
 */

#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>
#include <memory>

#include <gsl/gsl_math.h>
#include <gsl/gsl_deriv.h>

#include "../../lib_visual/gnuplot_iostream.hpp"

using namespace std;

double f(double x,  void* params= nullptr){
    return pow(x,10.0);
}

double df(double x){
    return 10*pow(x,9.0);
}

//2 puntos derecha
double d2_r(double x, double h){
    return (f(x+h) - f(x))/h;
}

//2 puntos izquierda
double d2_l(double x, double h){
    return (f(x) - f(x-h))/h;
}

//3 puntos central
double d3_c(double x, double h){
    return (f(x+h)-f(x-h))/(2*h);
}

double d5_c(double x, double h){
    return (f(x-2*h) - 8*f(x-h) + 8*f(x+h) - f(x+2*h))/(12*h);
}

//5 puntos, GSL
double d5_gsl_c(double x, double h){
    gsl_function F;
    memset(&F,0,sizeof(F));
    F.function = &f;

    double res, err;
    gsl_deriv_central(&F, x, h, &res, &err);
    return res;
}


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


int main(){

    t_list_plots plots;
    plots.resize(3);

    const double x = 3;
    for(double h = 10e-2; h > 10e-4; h -= 10e-4){
        plots[0].push_back(make_pair(h, d3_c(x,h) - df(x)));
        plots[1].push_back(make_pair(h, d5_c(x,h) - df(x)));
        plots[2].push_back(make_pair(h, d5_gsl_c(x,h) - df(x)));
    }

    show(plots);

    return 0;
}



