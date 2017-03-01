/*
 * Created on: Feb 11, 2017
 * Author: Dr. Yevgeniy Kolokoltsev
 *
 * Numerov algorithm is used to solve an initial value problem
 * for equation of type (see Koonin):
 *
 *      d2y(x)/dx2 + k(x)^2 y(x) = S(x)
 *
 * It permits to solve this differential equation from left to right (forwards)
 * and from right to left (backwards along the x-axis). The case of "-k^2"
 * can be reached with T = complex<..>.
 *
 * FWD initial values: y[2] = {y(x0), y(x0+h)}, initial_x = x0+h;
 * BWD initial values: y[2] = {y(x1-h), y(x1)}
 *
 */

#ifndef COMPPHYSFWK_NUMEROV_HPP
#define COMPPHYSFWK_NUMEROV_HPP

#include <iostream>
#include <tuple>
#include <array>
#include <cmath>
#include <type_traits>

using namespace std;

//some useful global types
enum class DIR{fwd = 1, bwd = -1};

template<typename T>
using f_ptr = T(*)(const T, const void*);

//selects N-th type from types list T
template<size_t N, typename... T>
using static_switch = typename tuple_element<N, std::tuple<T...> >::type;


template<typename T, f_ptr<T> f_k, f_ptr<T> f_s, DIR dir>
class Numerov {

private:
    static constexpr auto direction = dir == DIR::fwd? "forward" : "backward";

    static constexpr auto c = dir == DIR::fwd? 1 : 0;  //index of y[] that coincide with x
    static constexpr auto rl = dir == DIR::fwd? 0 : 1; //index of y[] that is left or right from x-center depending on direction

    static constexpr size_t t_hom_idx = f_k == nullptr ? 0 : 1;   //selector index for homogeneous solver
    static constexpr size_t t_inhom_idx = f_s == nullptr ? 0 : 1; //selector index for complete (inhomogeneous) solver

    //the most basic case: f_k == nullptr;
    struct EvalA {
        EvalA(T h, T x, void* params = nullptr): h{abs(h)*((int)dir)}, h2{h*h}, x{x}, params{params} {};

    public:
        inline void eval_next(array<T,2>& y){
            T yc = y[c];
            y[c] = nom(y)/denom();
            y[rl] = yc;
            x += h;
        }

        void set_x(T x){this->x = x;}
        T get_x(){return x;}
        virtual string get_config(){return string(direction) + " f_k(null)"; };

    protected:
        inline virtual T nom(const array<T,2>& y){ return ((T)24)*y[c]-((T)12.0)*y[rl]; };
        inline virtual T denom(){ return (T)12; };
        inline void update_x_neigh(){ x_rl = x-h; x_lr = x+h; }

        const T h, h2;
        const void* params;
        T x, x_rl, x_lr;
    public:

    };

    //if f_k is a valid function than we can modify EvalA
    struct EvalB : public EvalA{
        using Base = EvalA;
        EvalB(T h, T x, void* params = nullptr): Base(h, x, params){};

        inline void eval_next(array<T,2>& y){ Base::update_x_neigh(); Base::eval_next(y); }
        virtual string get_config(){return string(direction) + " f_k(...)"; };

    protected:
        inline virtual T nom(const array<T,2>& y){ return Base::nom(y) - Base::h2*(((T)10)*pow(f_k(Base::x, Base::params),2)*y[c] + pow(f_k(Base::x_rl, Base::params),2)*y[rl]); };
        inline virtual T denom(){ return EvalA::denom() + Base::h2*pow(f_k(Base::x_lr, Base::params),2); };
    };

    using T_HOM = static_switch<t_hom_idx,EvalA,EvalB>;

    //if f_S == nullptr, we have a homogeneous case, so T_HOM is not modified, here we simply adjust the complete configuration info
    struct EvalC : public T_HOM{
        using Base = T_HOM;
        EvalC(T h, T x, void* params = nullptr): Base(h, x, params){};

        virtual string get_config(){return Base::get_config() + " f_s(null)"; };
    };

    //if f_S is a valid function, we have an inhomogeneous case, and T_HOM shell be extended with S(x) function evaluation
    struct EvalD : public T_HOM{
        using Base = T_HOM;
        EvalD(T h, T x, void* params = nullptr): Base(h, x, params){};

        inline void eval_next(array<T,2>& y){ Base::update_x_neigh(); Base::eval_next(y); }
        virtual string get_config(){return Base::get_config() + " f_s(...)"; };

    protected:
        inline virtual T nom(const array<T,2>& y){return Base::nom(y) + Base::h2*(f_s(Base::x_rl, Base::params) + f_s(Base::x_lr, Base::params) + 10*f_s(Base::x, Base::params));}
    };

public:
    //this is the resulting solver engine, it contains ONLY those actions that are necessary for
    // particular-case computations
    using t_numerov = static_switch<t_inhom_idx,EvalC,EvalD>;
};

#endif //COMPPHYSFWK_NUMEROV_HPP
