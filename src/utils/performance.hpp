//
// Created by morrigan on 1/14/17.
//

#ifndef COMPPHYSFWK_PERFORMANCE_HPP
#define COMPPHYSFWK_PERFORMANCE_HPP

#include <ostream>
#include <chrono>
#include <type_traits>
#include <string>
#include <sstream>

using namespace std;
using namespace std::chrono;

template<typename T, typename UTS = milliseconds>
class ns_timer{
    using t_clock = steady_clock;

public:
    ns_timer(){
        if(is_same<UTS, nanoseconds>::value){
            units = "ns";
        }else if(is_same<UTS, microseconds>::value){
            units = "us";
        }else if(is_same<UTS, milliseconds>::value){
            units = "ms";
        }else if(is_same<UTS, seconds>::value){
            units = "s";
        }else if(is_same<UTS, minutes>::value){
            units = "min";
        }else if(is_same<UTS, hours>::value){
            units = "hrs";
        }else{
            units = "??";
        }
    }

    string dt() const{
        ostringstream os;
        os  << duration_cast<UTS>(t1 - t0).count() << units;
        return os.str();
    }

    inline void start(){t0 = t_clock::now();}
    inline void stop(){t1 = t_clock::now();}

public:
    T res;

private:
    t_clock::time_point t0, t1;
    string units;
};

template<typename T>
ostream& operator<<(ostream& os, const ns_timer<T>& timer)
{
    return os << "{" << timer.res << "} " << timer.dt();
}

#endif //COMPPHYSFWK_PERFORMANCE_H
