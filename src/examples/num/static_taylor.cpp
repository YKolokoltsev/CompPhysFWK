//
// Created by morrigan on 2/20/17.
//

#include <iostream>
#include <cmath>

#include "../../utils/performance.hpp"

using namespace std;

//Static Taylor series of Exp[x] centered at point 'a = num/denum'
template <int num, int denum>
double series_exp(double dx){
    constexpr double a = num/denum;
    constexpr double c0 = exp(a)*(1.0-a+a*a/2.0);
    constexpr double c1 = exp(a)*(1.0 - a);
    constexpr double c2 = exp(a)/2.0;

    return c0+c1*dx+c2*dx*dx;
}

//Dynamic case of the same series
double series_exp(double dx, double a){
    double c0 = exp(a)*(1.0-a+a*a/2.0);
    double c1 = exp(a)*(1.0 - a);
    double c2 = exp(a)/2.0;

    return c0+c1*dx+c2*dx*dx;
}

int main(int argc, char** argv){

    ns_timer<string> timer;
    double sum = 0;

    timer.res = "static method (fast)";
    timer.start();
    for(double dx = -0.5; dx <= 0.5; dx += 0.0001) sum +=  series_exp<5,2>(dx);
    timer.stop();
    cout << timer << "  result: " << sum << endl;

    sum = 0;
    timer.res = "dynamic method (slow)";
    timer.start();
    for(double dx = -0.5; dx <= 0.5; dx += 0.0001) sum +=  series_exp(dx,5/2);
    timer.stop();
    cout << timer << "  result: " << sum << endl;

    return 0;
}