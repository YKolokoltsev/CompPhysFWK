//
// Created by morrigan on 21/12/16.
//

#ifndef COMPPHYSFWK_FILTERS_H
#define COMPPHYSFWK_FILTERS_H

#include <vector>
#include <cmath>

using namespace std;

Signal rc_filter(double dt, double T, double RC = 1){
    size_t len = ceil(T/dt);
    vector<double> h(len);

    int i = 0;
    for(double t = -T; t < 0; t += dt, i++)
        h[i] = exp(t/RC);

    Signal filter(dt,-T,move(h));
    filter.normalize();
    return filter;
}

Signal inverse_rc_filter(double dt, double T, double RC = 1){
    size_t len = ceil(T/dt);
    vector<double> h(len);

    double wd = (2*M_PI/dt)/10;
    double w = wd/2;

    int i = 0;
    for(double t = -T/2; t < T/2; t += dt, i++){
        if(abs(t) < dt){
            h[i] = (wd + pow(wd,3)*RC*t/12 - pow(wd,3)*t*t/24 - pow(wd,5)*pow(t,3)*RC/480);
        }else{
            h[i] =  ( 2*(t+RC)*sin(w*t) - t*wd*RC*cos(w*t) )/pow(t,2);
        }
    }

    Signal filter(dt,-T/2,move(h));
    filter.normalize();
    return filter;
}

#endif //COMPPHYSFWK_FILTERS_H
