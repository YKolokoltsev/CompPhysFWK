//
// Created by morrigan on 9/01/17.
//

#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_integration.h>

#include "../../utils/performance.hpp"

using namespace std;
using namespace std::chrono;

const size_t N = 10000;
//[x0,x1] shell give analytical zero integral
const double x0=0, x1 = 2*M_PI, dx=(x1-x0)/(double)N;

double f(double x, void* params = nullptr){return sin(x);}

ns_timer<double> stl_accumulate(){
    ns_timer<double> timer;
    vector<double> y(N);

    auto it_y = y.begin();
    for(double cx = x0; cx < x1; cx += dx){ *it_y = f(cx); it_y++;}

    timer.start();
    timer.res = accumulate(y.begin(), y.end(), 0.0);
    timer.stop();

    return timer;
}

ns_timer<double> c_style_sum(){
    ns_timer<double> timer;
    double total = 0;
    double y[N];

    for(size_t i = 0; i < N; i++) y[i] = f((double)i*dx);

    timer.start();
    for(size_t i = 0; i < N; i++) total += y[i];
    timer.stop();
    timer.res = total;

    return timer;
}

ns_timer<double> analytic_step_total(){
    ns_timer<double> timer;
    double total = 0;

    timer.start();
    for(double cx = x0; cx < x1; cx += dx) total += f(cx);
    timer.stop();
    timer.res = total;

    return timer;
}


ns_timer<double> linear_data_interpolation(){

    ns_timer<double> timer;
    vector<double> x(N),y(N);

    auto it_x = x.begin(), it_y = y.begin();
    for(double cx = x0; cx < x1; cx += dx){
        *it_x = cx; it_x++;
        *it_y = f(cx); it_y++;
    }

    shared_ptr<gsl_interp> interp(gsl_interp_alloc(gsl_interp_linear,N),
                                  [&](gsl_interp* p_ctx) {gsl_interp_free(p_ctx);});
    gsl_interp_init(interp.get(), x.data(), y.data(),N);
    shared_ptr<gsl_interp_accel> acc(gsl_interp_accel_alloc (), [&](gsl_interp_accel* pa){ gsl_interp_accel_free(pa);});


    timer.start();
    timer.res = gsl_interp_eval_integ (interp.get(), x.data(), y.data(), x.front(), x.back(), acc.get());
    timer.stop();

    return timer;
}

ns_timer<double> numeric_integration(){
    ns_timer<double> timer;
    double epsabs = abs(linear_data_interpolation().res);
    gsl_function F{&f,nullptr};

    double res, abserr;
    size_t evals;

    timer.start();
    gsl_integration_qng (&F, x0, x1, epsabs, 0, &res, &abserr, &evals);
    timer.stop();
    timer.res = res;

    return timer;
}

int main(int argc, char** argv){
    cout << endl;
    cout << "GSL Linear interpolation (" << N << " pts) : " << linear_data_interpolation() << endl;
    cout << "GSL Nonuniform integration 5-th order " << numeric_integration() << endl;
    cout << "Analytic step sum " << analytic_step_total() << endl;
    cout << "Stl accumulate " << stl_accumulate() << endl;
    cout << "C style sum " << c_style_sum() << endl;

}
