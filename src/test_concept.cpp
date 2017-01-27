/*
 * test_concept.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

//http://www.binaryconvert.com/result_float.html?decimal=051046049052050053055056

#include <bitset>
#include <memory>
#include <iostream>
#include <cfloat>
//#include <cmath>

using namespace std;

int main(){

    float x = 1;

    using t_ul = unsigned long;
    auto ul = (*reinterpret_cast<t_ul*>(&x));

    int Exp = (ul&0x7F000000) >> 24;

    int Mant = (ul&0x00FFFFFF);

    int shift = 1 - (1 << (4*8 - FLT_MANT_DIG - 2));

    cout << bitset<32>(*reinterpret_cast<unsigned long*>(&x)) << endl;
    cout << "Mant: " << bitset<32>(Mant) << " " << Mant << endl;
    cout << "Exp: " << bitset<32>(Exp) << " " << Exp << endl;
    cout << "shift: " << shift << endl;


	return 0;
}
