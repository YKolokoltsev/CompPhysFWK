//
// Created by morrigan on 22/12/16.
//

#ifndef COMPPHYSFWK_SIGNAL_H
#define COMPPHYSFWK_SIGNAL_H

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <list>

using namespace std;

class Signal{
public:
    //simple constructor
    Signal(double dt):
            dt{dt}, i0_t{0}, data{} {};

    //init constructor, sell be called with move()
    Signal(double dt, double i0_t, vector<double>&& data):
            dt{dt}, i0_t{i0_t}, data{data} {};

    //copy constructor
    Signal(const Signal& s):
            dt{s.dt}, i0_t{s.i0_t}, data{s.data} {};

    //copy assignment
    Signal& operator=(const Signal& s){
        dt = s.dt; i0_t = s.i0_t; data = s.data;
        return *this;
    }

    //move constructor
    Signal(Signal&& s):
            dt{s.dt}, i0_t{s.i0_t}, data{move(s.data)} {};

    //move assignment
    Signal& operator=(Signal&& s){
        dt = s.dt; i0_t = s.i0_t; data = move(s.data);
    }

public:
    vector<pair<double,double>> get(){
        size_t len = data.size();
        vector<pair<double,double>> s(len);

        for(size_t i = 0; i < len; i++)
            s[i] = make_pair(i*dt + i0_t,data[i]);

        return s;
    }

    double energy(){
        double e = 0;
        for(auto y : data) e += y*y*dt;
        return e;
    }

    double norm(){
        return sqrt(energy());
    }

    double normalize() {
        auto scale = norm();
        for (auto &y : data) y /= scale;
    }

    Signal convolve(const Signal& h){
        if(h.dt != dt) throw runtime_error("can't convolve distinct sampling rates");
        Signal out(dt);
        out.data.resize(data.size() + h.data.size());

        for(int k = 0; k < data.size(); k++)
            for(int m = k; m < h.data.size() + k; m++){
                out.data.at(m) += data.at(k)*h.data.at(m-k)*dt;
            }

        out.i0_t = i0_t-h.data.size()*dt;

        return out;
    }

    inline double get_dt(){return dt;}
    inline double get_i0_t(){return i0_t;}

private:
    double dt; //sampling period
    double i0_t; //time of zero index

    vector<double> data;
};

#endif //COMPPHYSFWK_SIGNAL_H
