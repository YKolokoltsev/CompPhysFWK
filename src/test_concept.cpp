/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */
#include <iostream>
#include <cmath>
#include <stdlib.h>

using namespace std;

const int lp = 2;
const int lq = 4;
constexpr int L = lp + lq;

int main(int argc, char** argv){

   int D_min = 0, D_max = 0;
   for(int Sp = 0 ; Sp <= 100; Sp++){
       int I = Sp + rand() % (2*L) - L;
       int I_m = div(I,2*L).rem;
       int Sp_m = div(Sp,2*L).rem;

       int d_r = Sp_m - I_m;
       int d_q = d_r == L ? 0 : div(d_r,L).quot;
       int D = d_q*2*L-d_r;

       if(D_min > D) D_min = D;
       if(D_max < D) D_max = D;
   }


    int size = 3;
    D_min = size; D_max = 0;
    for(int i = 0; i < 1000; i++){
        int idx = (rand() % (int)(size));
        if(D_min > idx) D_min = idx;
        if(D_max < idx) D_max = idx;
    }

    cout << D_min << " " << D_max << endl;

    return 0;
}