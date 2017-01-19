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
#include <gsl/gsl_integration.h>

#include "gnuplot-iostream.h"

using namespace std;

const int N = 1000;
const double r0 = 0.1, r1 = 8.0, dr = (r1-r0)/N;

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

    inline double y(double x){return gsl_interp_eval (itp_y.ctx.get(), X.data(), Y.data(), x, itp_y.acc.get());}
    inline double dy(double x){return gsl_interp_eval (itp_dy.ctx.get(), X.data(), DY.data(), x, itp_dy.acc.get());}

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
    dydr[1] = y[0]*l*(l+1)/pow(r,2.0);
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

    X[0] = r0; X[N] = r1;

    return t_triple{X,Y,DY};
}

double wronskian(t_solution& Y1, t_solution& Y2){
    vector<double> wskn_ensemble(Y1.N);
    for(int i = 0; i < Y1.N; i++){
        double x = Y1.X[i];
        wskn_ensemble[i] = Y2.y(x)*Y1.dy(x) - Y1.y(x)*Y2.dy(x);
    }
    double w = gsl_stats_mean(wskn_ensemble.data(), 1, wskn_ensemble.size());
    double var = gsl_stats_variance(wskn_ensemble.data(), 1, wskn_ensemble.size());

    cout << "Wronskian is " << w << " and has an error of " << abs(100.0*var/w) << " %" << endl;

    return w;
}

double green_function(double x, double s, t_solution& Y1, t_solution& Y2, double Vo = 1){
    if(x < s) return Y1.y(x)*Y2.y(s)/Vo;
    return Y2.y(x)*Y1.y(s)/Vo;
}

using t_kernel_params = tuple<double /*x*/, t_solution&/*Y1*/, t_solution&/*Y2*/, double/*Vo*/>;

double kernel(double r, void* params = nullptr){
    t_kernel_params* p = (t_kernel_params*) params;
    return green_function(get<0>(*p)/*x*/, r, get<1>(*p)/*Y1*/,
                          get<2>(*p)/*Y2*/, get<3>(*p)/*Vo*/)*exp(-r)/(8*M_PI);
}

double psi(double x, t_solution& Y1, t_solution& Y2, double Vo = 1){

    t_kernel_params params{x, Y1, Y2, Vo};

    gsl_function F{&kernel,&params};
    double res, abserr;
    size_t neval;

    gsl_integration_qng (&F, r0, r1, 1e-2, 0, &res, &abserr, &neval);
    cout << "Integration: x(" << x << "); err(" << (100.0*abserr/res) << "%); neval(" << neval << ");" << endl;
    return res;
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
    t_solution Y1(evaluate_cauchy<eval_way::fwd>({0, 1}, d));
    t_solution Y2(evaluate_cauchy<eval_way::bwd>({0, -1}, d));


    double Vo = wronskian(Y1, Y2);
    const size_t SN = 50;
    Gnuplot gp;

#if 1
        //Plot  Green's Function
        gp << " $wgrid << EOD " << endl;
        for (double x = r0; x <= r1; x += 3 * (r1 - r0) / SN) {
            for (double s = r0; s <= r1; s += (r1 - r0) / SN)
                gp << s << " " << x << " " << green_function(x, s, Y1, Y2, Vo) << endl;
            gp << endl << endl;
        }
        gp << "EOD" << endl;
        gp << "splot '$wgrid' with lines palette notitle" << endl;
#else
        //plot ingomogeneous solution
        gp << " $psi << EOD " << endl;
        for (double r = r0; r <= r1; r += dr)
            gp << r << " " << psi(r, Y1, Y2, Vo) << endl;
        gp << "EOD" << endl;
        gp << "plot '$psi' with lines t 'psi'" << endl;
#endif

    return 0;
}
