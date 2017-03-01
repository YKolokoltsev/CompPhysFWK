/*
 * schrodinger_shoot.cpp
 *
 *  Created on: Feb 12, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 *
 * Here we attempt to find eigenvalues of the 1-D Schrodinger equation by
 * the shooting method (Koonin p.67). Here we use a well known potential V(x) = x^2,
 * exponential decay boundary conditions and compare the results with analytical solution.
 * 
 * As long as k^2 takes negative values in classically prohibited region, we shell use
 * complex numbers.
 */

#include <iostream>
#include <complex>
#include <memory>

#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>

#include "../../lib_num/numerov.hpp"
#include "../../lib_visual/gnuplot_iostream.hpp"
#include "../../lib_num/root_localize.hpp"

using namespace std;

/**
 * Definition of the physical functions
 */

//potential function
template<typename T>
T V(const T x, void* params = nullptr){
    return (T)pow(x,2.0);
}

//derivative of the potential function (needed for roots localization)
template<typename T>
T dV(const T x, void* params = nullptr){
    return (T)2.0*x;
}

//wave number function, used by Numerov algorithm
template<typename T>
T k(const T x, const void* params){
    T E = *(static_cast<const T*>(params));
    return sqrt(E - V(x));
}

template<typename T>
T k_zeros(T x, void* params){
    T E = *(static_cast<T*>(params));
    return E - V<T>(x);
}

template<typename T>
T dk_zeros(T x, void* params){
    return -dV<T>(x);
}

//used by GSL gradient root finding routines, just a stub in this program
//but can rise efficiency in the most cases, see GSL reference
template<typename T>
void fdk_zeros(T x, void * params, T* f, T* df){
    *f = k_zeros<T>(x, params);
    *df = dk_zeros<T>(x, params);
}

/**
 * Definition of some helpful types
 */

//data points struct (to simplify plotting)
struct t_datapts{
    vector<pair<double,double>> pts;
    string cmd;
};

//1-d boundaries struct, this struct is a helpful storage for
//those solutions that touch (necessarily for 1-d case) two
//boundary conditions
struct t_bounds{
    vector<pair<double,double>> left;
    vector<pair<double,double>> right;
};

//negative wave number support in Numerov require complex values (shitty)
using Cmplx = complex<double>;

//function type definition is used for functors construction
template<typename T>
using t_func = T(*)(T, void*);

//on the base of this functor template we can convert any function template with type
//'t_func' into a functor. This functor type is a template and this is
//the only reason why it is needed. In the interval root localization
//algorithm the same functions has to be called on intervals as well as for plain types
//so it is wise to hide these details inside of algorithm and let it change
//function type by it's own.
template<typename T,  t_func<T> F>
struct tmpl_functor{
    T operator()(T x, void* params) const {
        return F(x,params);
    }
};

/**
 * Some functors are needed for interval algorithm
 */

//root finder functor (needed for Newton interval method)
template<typename T>
using t_ftor_k_zeros = tmpl_functor<T,k_zeros>;

//analytic differential for root localization (needed for Newton interval method)
template<typename T>
using t_ftor_dk_zeros = tmpl_functor<T,dk_zeros>;


/**
 * Eigenvalue problem parameters
 */

//complete interval of interest
const double x0{-5}, x1{5};
//number of points
const size_t N = 500;
//integration step
const double h = (x1 - x0)/N;
//root finding precicion
const double eps = 0.00001;
const

list<double> find_zeros(double E){
    list<double> z;

    //first, get intervals with roots localized inside
    t_interval<double> search_int{x0,x1};
    t_interval<double> E_int{E,E};
    auto locs = localize_roots<double,t_ftor_k_zeros,t_ftor_dk_zeros>(search_int, &E_int);
    cout << "found " << locs.size() << " intervals" << endl;

    //a GSL root finder require this struct to be filled with
    //functions and it's parameters
    //cant't use constexpr here because of &E, but in the
    //particular case when functions should not be parameterized
    //this is a good place to fill this struct just once at a compile time
    gsl_function_fdf FDF{
            .f = k_zeros<double>,
            .df = &dk_zeros<double>,
            .fdf = &fdk_zeros<double>,
            .params = &E
    };

    //GSL is a C library, and sometymes require a permanent context
    //inside of its algorithms... this is the analog of C++ classes (encapsulation principle)
    //however these contexts would require it's explicit release (free/delete operators)
    //this is an old and buggy practice, so we use smart poiters destructors for this
    //purpose
    std::shared_ptr<gsl_root_fdfsolver> s(
            gsl_root_fdfsolver_alloc(gsl_root_fdfsolver_newton),
            [](gsl_root_fdfsolver* p){gsl_root_fdfsolver_free(p);}
    );

    //now in each interval found by 'localize_roots' we can start a gradient root search
    //without worrying about convergence (the only case when function extrema can appear inside an interval
    // is when function touches zero, however in this case the interval will be nearly zero)

    for(auto& loc : locs){
        //shell never happen
        if(empty(loc)) continue;

        //the function extrema conisides with it's zero (handeled by 'localize_roots')
        if(width(loc) < eps){
            z.push_back(median(loc));
            continue;
        }

        //at any other case we make a gsl_fdf iterative process
        gsl_root_fdfsolver_set(s.get(),&FDF,median(loc));
        double root;
        do{
            root = gsl_root_fdfsolver_root(s.get());
            gsl_root_fdfsolver_iterate(s.get());
        }while(gsl_root_test_delta(root, gsl_root_fdfsolver_root(s.get()), eps,0.0) != GSL_SUCCESS);
        z.push_back(gsl_root_fdfsolver_root(s.get()));
    }

    //sort the roots so it be easier to construct the Numerov integration sequence
    z.sort();

    return z;
}

double asymptotic_exp(double x, double dx,  double E){
    Cmplx cE{E,0.0};
    gsl_function F{[](double x, void* params){
         return exp(-imag(k(Cmplx(x,0),params))*abs(x));
    },&cE};

    double res, abserr;
    size_t evals;

    gsl_integration_qng (&F, x, x+dx, 0, 1e-6, &res, &abserr, &evals);
    return res;
}



/**
 *  In this function we find two solutions that satisfy
 *  psi = 0 at +infty (right) and -infty (left)
 *
 *  xl - is the left point at which we prefer to stop execution of the Numerov
 *  iteration for the left bound, xr - the same for the right.
 *
 *  it is supposed that intervals [-infty, xl] and [xr, +infty]
 *  are classically prohibited regions, so we can use wave equation
 *  asymptotics exp(-k x) to convert an infinite boundary condition to
 *  finite, concatenating evaluation with this exponent.
 */

t_bounds solve_bounds(double xl, double xr, double E){
    t_bounds bounds;

    // some useful intervals
    double inf = std::numeric_limits<double>::infinity();
    t_interval<double> int_l{-inf,xl}, int_r{xr, inf}, int_E{E,E};
    t_interval<double> int_pos{eps,inf};

    // first check that [-infty, xl] and [xr, +infty] are classically prohibited
    if(!empty(intersect(k_zeros(int_l,&int_E),int_pos)) || !empty(intersect(k_zeros(int_r,&int_E),int_pos)))
        throw runtime_error("oscillation on boundary intervals detected");


    // each Numerov iteration require two points as an initial condition;
    // when the iteration has already passed this points will be updated
    // and than, can be used for the next iteration (see numerov.hpp)
    // however, the first iteration require these points to be set manually -
    // an initial condition.
    array<Cmplx,2> psi;
    Cmplx mk; //median k at starting interval left: {x-h,x},  right: {x, x+h}

    // forbidden region happens when k^2 < 0, so we have to work with complex
    // numbers within Numerov, however, the resulting Psi will not contain
    // an imaginary part
    Cmplx cE = Cmplx{E,0};

    // create Numerov integrators
    Numerov<Cmplx,k<Cmplx>,nullptr,DIR::bwd>::t_numerov bwd(Cmplx(h,0), Cmplx(xl-1.0,0), &cE);
    Numerov<Cmplx,k<Cmplx>,nullptr,DIR::fwd>::t_numerov fwd(Cmplx(h,0), Cmplx(xr+1.0,0), &cE);




    // left boundary
    mk = (k(bwd.get_x()-(Cmplx)h, &cE) + k(bwd.get_x(), &cE))/2.0;
    psi = {exp(Cmplx(0,1)*mk*h), Cmplx{1,0}};



    /*do{
        bounds.left.push_back(make_pair(real(bwd.get_x()),real(psi[1])));
        bwd.eval_next(psi);
    }while(real(bwd.get_x()) > x0 && abs(psi[0]) > eps);*/

    for(double x = xl; x > x0; x -= h){
        //mk = (k(bwd.get_x()-(Cmplx)h, &cE) + k(bwd.get_x(), &cE))/2.0;
        mk = k((Cmplx)bwd.get_x(), &cE);
        bounds.left.push_back(make_pair(x,120.0*real( exp(-Cmplx(0,1)*mk*x/1.57079)  )));
    }

    reverse(bounds.left.begin(),bounds.left.end());

    // right boundary
    bwd.set_x(Cmplx(xl-1.0,0));
    psi = {asymptotic_exp(real(bwd.get_x())-h,h,E),  exp(-Cmplx(0,1)*k(bwd.get_x(),&cE)*bwd.get_x())  };
    cout << psi[0] << " " << psi[1] << endl;
    do{
        bounds.right.push_back(make_pair(real(bwd.get_x()),real(psi[1])));
        bwd.eval_next(psi);
    }while(real(bwd.get_x()) > x0 && abs(psi[0]) > eps);
    reverse(bounds.right.begin(),bounds.right.end());

    return move(bounds);
}

int main(int argc, char** argv){

    //list of objects to plot
    list<t_datapts> plot;
    vector<pair<double,double>> pts;

    double E = 3.0;

    //find zeros of the energy function
    auto zeros = find_zeros(E);
    for(auto x : zeros) pts.push_back(make_pair(x,V(x)));
    plot.push_back({move(pts)," '-' w p ls 7 notitle"});

    //take two boundary zeros and get boundary tails
    auto bounds = solve_bounds(zeros.front(), zeros.back(), E);
    plot.push_back({bounds.left," '-' with lines title 'l_b' "});
    plot.push_back({bounds.right," '-' with lines title 'l_r' "});


    using t_fwd = Numerov<Cmplx,k<Cmplx>,nullptr,DIR::fwd>::t_numerov;
    using t_bwd = Numerov<Cmplx,k<Cmplx>,nullptr,DIR::bwd>::t_numerov;

    //current two pints of psi function (used as initial values at each Numerov iteration)
    array<Cmplx,2> psi;
    //energy value
    Cmplx cE = Cmplx{E,0};

    //forward integrator
    t_fwd num_fwd(Cmplx(h,0), Cmplx(x0+h,0), &cE);

    psi = {Cmplx{0,0},Cmplx{0.0001,0}};
    pts.resize(N);
    for(size_t i = 0; i < N; i++) {
        pts.at(i) = make_pair(real(num_fwd.get_x()), real(psi[1]));
        num_fwd.eval_next(psi);
    }
    plot.push_back({move(pts)," '-' with lines title 'fwd'"});

    //backward integrator
    /*t_bwd num_bwd(h,num_fwd.get_x()-h, &E);

    psi = {T{3,0},T{0,0}};
    pts.resize(N);
    for(size_t i = 0; i < N; i++) {
        pts.at(i) = make_pair(real(num_bwd.get_x()), real(psi[0]));
        num_bwd.eval_next(psi);
    }
    plot.push_back({move(pts)," '-' with lines title 'bwd'"});*/

    //potential
    pts.resize(N);
    for(float i = 0; i < N; i++) pts.at(i) = make_pair(real(x0+i*h),real(V(x0+i*h)));
    plot.push_back({move(pts)," '-' with lines notitle"});

    //energy
    pts.resize(2);
    pts.at(0) = make_pair(real(x0),real(cE));
    pts.at(1) = make_pair(real(x1),real(cE));
    plot.push_back({move(pts)," '-' with lines notitle"});


    //Plotting results
    Gnuplot gp;
    if(!plot.empty()){
        gp << "plot ";
        for(auto it = plot.begin(); it != plot.end(); it++){
            gp << it->cmd;
            if(it != --plot.end()) gp << ","; else gp << endl;
        }
        for(auto& line : plot) gp.send1d(line.pts);
    }

    return 0;
}
