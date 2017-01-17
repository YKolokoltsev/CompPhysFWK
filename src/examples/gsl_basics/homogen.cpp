//
// Created by morrigan on 1/15/17.
//

#include <iostream>
#include <cmath>
#include <memory>
#include <vector>

#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_statistics_double.h>

#include "../../utils/gnuplot_iostream.hpp"

using namespace std;

const int N = 1000;
const double r0 = 0.1, r1 = 6.0, dr = (r1-r0)/N;

using t_triple = tuple<vector<double>,vector<double>,vector<double>>; //x, y, y'
enum class eval_way{fwd,bwd};

struct t_solution{
    t_solution(t_triple&& s): X(move(get<0>(s))), Y(move(get<1>(s))), DY(move(get<2>(s))) {
        N = X.size();
        itp_y.init(X,Y,N);
        itp_dy.init(X,DY,N);
    };

    struct t_interp{
        void init(const vector<double>& x, const vector<double>& y, const size_t N)
        {
            ctx = shared_ptr<gsl_interp>(gsl_interp_alloc(gsl_interp_linear,N), [](gsl_interp* p_ctx) {gsl_interp_free(p_ctx);});
            gsl_interp_init(ctx.get(), x.data(), y.data(), N);
            acc = shared_ptr<gsl_interp_accel>(gsl_interp_accel_alloc(), [](gsl_interp_accel* pa){ gsl_interp_accel_free(pa);});
        };

        shared_ptr<gsl_interp> ctx;
        shared_ptr<gsl_interp_accel> acc;
    };

    t_interp itp_y, itp_dy;
    vector<double> X,Y,DY;
    size_t N;
};

//first order regular differential equation system
int f(double r, const double y[], double dydr[], void *params = nullptr){
    if(r < 0) return GSL_EFAULT;
    //y[0] = psi; y[1] = d psi/dr;
    const double l = 1;
    dydr[0] = y[1];
    if(abs(pow(r,2.0) - y[0]) < 1e-3){
        dydr[1] = l*(l+1);
    }else{
        dydr[1] = y[0]*l*(l+1)/pow(r,2.0);
    }
    return GSL_SUCCESS;
}

//two way Cuachy evaluation with normalization
template<eval_way w>
inline t_triple evaluate_cauchy(vector<double> y, shared_ptr<gsl_odeiv2_driver> &d){

    constexpr struct it_i{int i0; int i1; double di;} ci = w == eval_way::fwd ? it_i{0,N+1,1} : it_i{N,-1,-1};
    constexpr struct it_r{double r0; double dr;} cr = w == eval_way::fwd ? it_r{r0,dr} : it_r{r1,-dr};

    vector<double> X(N+1), Y(N+1), DY(N+1);
    double norm = 0;

    int i = ci.i0;
    double r = cr.r0;
    gsl_odeiv2_driver_reset_hstart(d.get(),ci.di*1e-6);

    do{
        X[i] = r; Y[i] = y[0]; DY[i] = y[1];
        norm += y[0]*y[0];
        gsl_odeiv2_driver_apply(d.get(), &r, r+cr.dr, y.data());
        i += ci.di;
    }while(i != ci.i1);

    norm = sqrt(norm);
    i = ci.i0;
    do{
        Y[i] /= norm; DY[i] /= norm;
        i += ci.di;
    }while(i != ci.i1);

    return t_triple{X,Y,DY};
}

double wronskian(double x, const t_solution& Y1, const t_solution& Y2){

    double y1  = gsl_interp_eval (Y1.itp_y.ctx.get(), Y1.X.data(), Y1.Y.data(), x, Y1.itp_y.acc.get());
    double dy1 = gsl_interp_eval (Y1.itp_y.ctx.get(), Y1.X.data(), Y1.DY.data(), x, Y1.itp_dy.acc.get());
    double y2  = gsl_interp_eval (Y2.itp_y.ctx.get(), Y2.X.data(), Y2.Y.data(), x, Y2.itp_y.acc.get());
    double dy2 = gsl_interp_eval (Y2.itp_y.ctx.get(), Y2.X.data(), Y2.DY.data(), x, Y2.itp_dy.acc.get());

    return y1*dy2 - y2*dy1;
}

int main(){

    //initialize differential equations solver
    gsl_odeiv2_system sys = {f, nullptr, 2, nullptr};
    shared_ptr<gsl_odeiv2_driver> d(
            gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45, 1e-6,1e-6,0.0),
            [](gsl_odeiv2_driver* pd){gsl_odeiv2_driver_free(pd);}
    );

    //make two evaluations: forward and backward
    //t_solution constructor interpolate data automatically
    t_solution Y1(evaluate_cauchy<eval_way::fwd>({1, 0}, d));
    t_solution Y2(evaluate_cauchy<eval_way::bwd>({0, -1}, d));

    //show results
    t_list_plots plots(2);
    for(int i = 0; i < Y1.N; i++)plots[0].push_back(make_pair(Y1.X[i],Y1.Y[i]));
    for(int i = 0; i < Y2.N; i++)plots[1].push_back(make_pair(Y2.X[i],Y2.Y[i]));
    show(plots);

    vector<double> wskn_ensemble(Y1.N-2);
    for(int i = 1; i < Y1.N-1; i++) wskn_ensemble[i-1] = wronskian(Y1.X[i],Y1,Y2);
    cout << endl << "Wronskian statistics:" << endl;
    cout << "mean = "     <<  gsl_stats_mean(wskn_ensemble.data(), 1, wskn_ensemble.size()) << endl;
    cout << "variance = " <<  gsl_stats_variance(wskn_ensemble.data(), 1, wskn_ensemble.size()) << endl;
    cout << "max = "      <<  gsl_stats_max(wskn_ensemble.data(), 1, wskn_ensemble.size()) << endl;
    cout << "min = "      <<  gsl_stats_min(wskn_ensemble.data(), 1, wskn_ensemble.size()) << endl;

    return 0;
}
