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

using T = float;

//dummy function that returns zero, used to test Numerov engine performance
T f(const T){ return 0; }

int main(int argc, char** argv){

    ns_timer<string> timer;
    const T h = 1.0;
    const size_t N = 1000;
    array<T,2> y{0,0.101};


    Numerov<T,nullptr,nullptr,DIR::fwd>::t_numerov a00(h,0);
    Numerov<T,nullptr,f,DIR::fwd>::t_numerov       a01(h,0);
    Numerov<T,f,nullptr,DIR::fwd>::t_numerov       a10(h,0);
    Numerov<T,f,f,DIR::fwd>::t_numerov             a11(h,0);

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

