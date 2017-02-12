/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <iostream>

#include "../../lib_num/numerov.hpp"
#include "../../utils/performance.hpp"

using namespace std;

//dummy function that returns zero, used to test Numerov engine performance
double f(const double){
    return 0;
}

int main(int argc, char** argv){

    ns_timer<string> timer;
    const double h = 1;
    const size_t N = 1000;
    array<double,2> y{0,0.101};

    Numerov<nullptr,nullptr,DIR::fwd>::t_numerov a00(h);
    Numerov<nullptr,f,DIR::fwd>::t_numerov       a01(h);
    Numerov<f,nullptr,DIR::fwd>::t_numerov       a10(h);
    Numerov<f,f,DIR::fwd>::t_numerov             a11(h);

    timer.start();
    for(size_t i = 0; i < N; i++) a00.eval_next(y);
    timer.stop();
    timer.res = a00.get_config();
    cout << timer << endl;

    timer.start();
    for(size_t i = 0; i < N; i++) a01.eval_next(y);
    timer.stop();
    timer.res = a01.get_config();
    cout << timer << endl;

    timer.start();
    for(size_t i = 0; i < N; i++) a10.eval_next(y);
    timer.stop();
    timer.res = a10.get_config();
    cout << timer << endl;

    timer.start();
    for(size_t i = 0; i < N; i++) a11.eval_next(y);
    timer.stop();
    timer.res = a11.get_config();
    cout << timer << endl;
}

