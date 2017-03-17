/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */
#include <iostream>
#include <cmath>
#include <thread>

using namespace std;

struct MEM{
    int a,b,c,d,e;
} m;

ostream &operator<<(ostream& os, const MEM& x){
    os << "(" << x.a << "," << x.b << "," ;
    os << x.c << "," << x.d << "," << x.e << ")";
}

int run(){

    for(int i = 0; i < 1000; i++){
        ::this_thread::sleep_for(chrono::nanoseconds(1));
        if(m.a != m.b || m.b != m.d || m.c != m.d || m.d != m.e)
        cout << m << endl;
    }

    return 0;
}

int main(int argc, char** argv){

    thread a(&run);

    for(int i = 0; i < 1000; i++){
        m.a = i;
        m.b = i;
        m.c = i;
        ::this_thread::sleep_for(chrono::nanoseconds(1));
        m.d = i;
        m.e = i;
    }


    a.join();

    return 0;
}