//
// Created by morrigan on 9/01/17.
//

#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_integration.h>

#include "../../utils/performance.hpp"

using namespace std;

const size_t N = 1000;
//[x0,x1] shell give analytical zero
const double x0=0, x1 = 2*M_PI, dx=(x1-x0)/(double)N;

double f(double x, void* params = nullptr){return sin(x);}


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
    timer.res = gsl_integration_qng (&F, x0, x1, epsabs, 0, &res, &abserr, &evals);
    timer.stop();

    return timer;
}

int main(){

    cout << endl;
    cout << "Linear interpolation (" << N << " pts) : " << linear_data_interpolation() << endl;
    cout << "Nonuniform integration 5-th order " << numeric_integration() << endl;

}
