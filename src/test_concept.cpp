/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */
#include <iostream>
#include <cmath>

using namespace std;

template<typename T>
using t_func = T(*)(T&, void*);

template<typename T,  t_func<T> F>
struct tmpl_functor{
    T operator()(T& x, void* params) const {
        return F(x,params);
    }
};

template<typename T>
T f_impl(T& x, void* params){
    return ++x;
}

template<typename T>
using t_f = tmpl_functor<T,f_impl>;

int main(int argc, char** argv){

    t_f<double> m_f;

    double x = 1;
    cout << m_f(x, nullptr) << endl;

    return 0;
}