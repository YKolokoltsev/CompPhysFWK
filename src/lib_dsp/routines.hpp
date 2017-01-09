//
// Created by morrigan on 21/12/16.
//

#ifndef COMPPHYSFWK_ROUTINES_HPP
#define COMPPHYSFWK_ROUTINES_HPP

#include <vector>

#include "signal.hpp"

using namespace std;

vector<double> deconvolve(const vector<double>& h, const vector<double>& x){
    vector<double> y;
    vector<double> b;
    for(int i = 1; i < h.size(); i++) b.push_back(-h[i]);

    for(int k = 0; k < x.size(); k++){

        //	for(int m = 0; m < min(1.0*y.size()-1,1.0*b.size()); m++)

    }

    return y;
}

#endif //COMPPHYSFWK_ROUTINES_HPP
