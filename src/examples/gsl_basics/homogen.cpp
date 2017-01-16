//
// Created by morrigan on 1/15/17.
//

#include <iostream>
#include <cmath>
#include <memory>
#include <vector>

#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>

#include "../../utils/gnuplot_iostream.hpp"

using namespace std;

int f(double r, const double y[], double dydr[], void *params = nullptr){
    if(r < 0) return GSL_EFAULT;
    //y[0] = psi; y[1] = d psi/dr;
    //dydr[0] = d psi /dr; dydr[1] = d2 psi /dt2
    const double l = 3;
    dydr[0] = y[1];
    if(abs(pow(r,2.0) - y[0]) < 1e-3){
        dydr[1] = l*(l+1);
    }else{
        dydr[1] = y[0]*l*(l+1)/pow(r,2.0);
    }
    return GSL_SUCCESS;
}

int main(){

    gsl_odeiv2_system sys = {f, nullptr, 2, nullptr};
    shared_ptr<gsl_odeiv2_driver> d(
            gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45, 1e-6,1e-6,0.0),
            [](gsl_odeiv2_driver* pd){gsl_odeiv2_driver_free(pd);}
    );

    const double dt = 0.01;
    double t = 2, ti = t, t1 = 6.0;
    vector<double> y{1.0, 0.0};
    t_list_plots plots(1);

    do{
        ti += dt;
        if(gsl_odeiv2_driver_apply(d.get(), &t, ti, y.data()) != GSL_SUCCESS ){
            cerr << "an error occurred" << endl;
            break;
        };
        plots[0].push_back(make_pair(t, y[0]));
    }while(ti < t1);

    show(plots);

    return 0;
}
