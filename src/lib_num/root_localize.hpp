//
// Created by morrigan on 2/20/17.
//

#ifndef COMPPHYSFWK_ROOT_LOCALIZE_H
#define COMPPHYSFWK_ROOT_LOCALIZE_H

#include <functional>

#include <boost/numeric/interval.hpp>    // must be first for <limits> workaround
#include <ostream>

using namespace std;
using namespace boost;
using namespace numeric;
using namespace interval_lib;

template<typename T>
using t_interval = interval<T, policies<save_state<rounded_transc_opp<T>>, checking_base<T>>>;

template<typename T>
ostream &operator<<(ostream& os, const t_interval<T>& X){
    os << "(" << X.lower() << "," << X.upper() << ")";
}

//todo: touching roots do not fail, however are a bit inefficient
template <typename T = double, template<typename> typename f, template<typename> typename df>
list<t_interval<T>> localize_roots(t_interval<T> a, void* params){

    using t_I = t_interval<T>;

    f<double> mf;
    f<t_I> mF;
    df<t_I> mDF;

    size_t n_iterations = 0;

    list<t_I> clean,  dirty{a};

    while(!dirty.empty()){
        n_iterations++;

        auto X = dirty.front();
        dirty.pop_front();

        auto F = mF(X, params);
        auto DF = mDF(X, params);

        if(!zero_in(DF)){
            // the only question here: if X contain roots?
            if(zero_in(F)){
                cout << "Clean: " << X << endl;
                clean.push_back(X);
            }
        }else{
            // there are zeros inside DF so we need to split it:
            auto DF_lo = t_I(DF.lower(),0.0);
            auto DF_hi = t_I(0.0,DF.upper());

            //the only finite interval restriction
            auto x = median(X);

            // make two Newton iterations
            auto X1 = intersect(X,x - mf(x, params)/DF_lo);
            auto X2 = intersect(X,x - mf(x, params)/DF_hi);

            if(!empty(intersect(X1, X2))) cout << "has intersection after split" << endl;

            //and go to next iteration, if the results are not empty
            if(!empty(X1)) dirty.push_back(X1);
            if(!empty(X2)) dirty.push_back(X2);
        }

    }

    cout << "roots was localized after " << n_iterations << "  iterations" << endl;

    return clean;
}


#endif //COMPPHYSFWK_ROOT_LOCALIZE_H
