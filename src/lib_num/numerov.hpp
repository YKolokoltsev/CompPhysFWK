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
 * and from right to left (backwards along the x-axis)
 *
 * FWD initial values: y, yp
 * BWD initial values: yn, y
 *
 * where yn = y(x0 - h), y = y(x0), yp = y(x0 + h)
 */

#ifndef COMPPHYSFWK_NUMEROV_HPP
#define COMPPHYSFWK_NUMEROV_HPP

#include <tuple>
#include <array>
#include <cmath>

using namespace std;

//some useful global types
enum class DIR{fwd = 1, bwd = -1};
using f_ptr = double(*)(const double);

//selects N-th type from types list T
template<size_t N, typename... T>
using static_switch = typename tuple_element<N, std::tuple<T...> >::type;

template<f_ptr f_k, f_ptr f_s, DIR dir>
class Numerov {

private:
    static constexpr auto direction = dir == DIR::fwd? "forward" : "backward";

    static constexpr auto c = dir == DIR::fwd? 0 : 1;  //index of y[] that coincide with x
    static constexpr auto rl = dir == DIR::fwd? 1 : 0; //index of y[] that is left or right from x-center depending on direction

    static constexpr size_t t_hom_idx = (f_k == nullptr) ? 0 : 1;   //selector index for homogeneous solver
    static constexpr size_t t_inhom_idx = (f_s == nullptr) ? 0 : 1; //selector index for complete (inhomogeneous) solver

    //the most basic case: f_k == nullptr;
    struct EvalA{
        EvalA(double h, double x = 0.0): h{abs(h)*((int)dir)}, h2{h*h}, x{x}{};

    public:
        inline void eval_next(array<double,2>& y){
            double yc = y[rl];
            y[rl] = nom(y)/denom();
            y[c] = yc;
            x += h;
        }

        void set_x(double x){this->x = x;}
        double get_x(){return x;}
        static string get_config(){return string(direction) + " f_k(null)"; };

    protected:
        inline virtual double nom(const array<double,2>& y){ return 24.0*y[c]-12.0*y[rl]; };
        inline virtual double denom(){ return 12; };
        inline void update_x_neigh(){x_rl = x+h; x_lr = x-h;}

        const double h, h2;
        double x, x_rl, x_lr;
    };

    //if f_k is a valid function than we can modify EvalA
    struct EvalB : public EvalA{
        using Base = EvalA;
        EvalB(double h, double x = 0.0): Base(h, x){};

        inline void eval_next(array<double,2>& y){ Base::update_x_neigh(); Base::eval_next(y); }
        static string get_config(){return string(direction) + " f_k(...)"; };

    protected:
        inline virtual double nom(const array<double,2>& y){ return Base::nom(y) - Base::h2*(10*pow(f_k(Base::x),2)*y[c] - pow(f_k(Base::x_rl),2)*y[rl]); };
        inline virtual double denom(){ return EvalA::denom() + Base::h2*pow(f_k(Base::x_lr),2); };
    };

    using T_HOM = static_switch<t_hom_idx,EvalA,EvalB>;

    //if f_S == nullptr, we have a homogeneous case, so T_HOM is not modified, he we simply adjust the complete configuration info
    struct EvalC : public T_HOM{
        using Base = T_HOM;
        EvalC(double h, double x = 0.0): Base(h, x){};

        static string get_config(){return Base::get_config() + " f_s(null)"; };
    };

    //if f_S is a valid function, we have an inhomogeneous case, and T_HOM shell be extended with S(x) function evaluation
    struct EvalD : public T_HOM{
        using Base = T_HOM;
        EvalD(double h, double x = 0.0): Base(h, x){};

        inline void eval_next(array<double,2>& y){ Base::update_x_neigh(); Base::eval_next(y); }
        static string get_config(){return Base::get_config() + " f_s(...)"; };

    protected:
        inline virtual double nom(const array<double,2>& y){return Base::nom(y) + Base::h2*(f_s(Base::x_rl) + f_s(Base::x_lr) + 10*f_s(Base::x));}
    };

public:
    //this is the resulting solver engine, it contains ONLY those actions that are necessary for
    // particular-case computations
    using t_numerov = static_switch<t_inhom_idx,EvalC,EvalD>;
};

#endif //COMPPHYSFWK_NUMEROV_HPP
