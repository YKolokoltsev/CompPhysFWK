/*
 * numerov.cpp
 *
 *  Created on: Feb 11, 2017
 *      Author: Dr. Yevgeniy Kolokoltsev
 */
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <bitset>
#include <memory>
#include <vector>

using namespace std;

struct A{
    virtual string get_name(){return "class A";}
};

struct B : public A{
    virtual string get_name(){return ("class " + s);}
    void print_true(){cout << "TRUEEEE";}
    string s{"B"};
};

struct C : public A{

};

int main(int argc, char** argv){

    using t_ptr = shared_ptr<A>;
    vector<t_ptr> cont;

    cont.push_back(t_ptr(new A()));
    cont.push_back(t_ptr(new B()));
    cont.push_back(t_ptr(new C()));

    for(auto & x : cont){
        cout << x->get_name() << endl;
        //if(x->get_name() == "class B"){
            ((B*)((void*)x.get()))->print_true();
        //}
    }

    return 0;
}