//
// Created by morrigan on 21/12/16.
//

#ifndef COMPPHYSFWK_SIGNALS_H
#define COMPPHYSFWK_SIGNALS_H

#include <vector>
#include <cmath>

#include "signal.hpp"

using namespace std;

Signal gaussian_pulse(double dt, double T, double sigma = 1){
    size_t len = ceil(T/dt);
    vector<double> signal; signal.resize(len);

    int i = 0;
    for(double t = -T/2; t < T/2; t += dt, i++)
        signal[i] = exp(-pow(t/sigma,2)/2);

    Signal out(dt,-T/2,move(signal));
    out.normalize();

    return out;
}

Signal chirp_pulse(double dt, double T, double alpha = 1, double w0 = 0, double fi = 0){

    vector<double> signal;
    signal.resize(ceil(T/dt));

    int i = 0;
    for(double t = -T/2; t < T/2; t += dt, i++)
        signal[i] = sin(pow(alpha*t,2) + w0*t + fi);

    return Signal(dt, -T/2, move(signal));

}

#endif //COMPPHYSFWK_SIGNALS_H
