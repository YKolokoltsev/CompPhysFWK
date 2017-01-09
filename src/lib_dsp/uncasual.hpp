/*
 * test_concept.cpp
 *
 *  Created on: Dic 21, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#ifndef COMPPHYSFWK_UNCASUAL_HPP
#define COMPPHYSFWK_UNCASUAL_HPP

#include <complex>
#include <memory>
#include <vector>

#include <gsl/gsl_poly.h>

using namespace std;

using t_complex = complex<double>;

struct UncasualFilter{
    vector<double> transverse;
    vector<double> recursive;
};

vector<t_complex> find_roots(const vector<double>& coefficients){
    size_t len = coefficients.size();
    vector<double> z; z.resize((len-1)*2);

    shared_ptr<gsl_poly_complex_workspace> w(gsl_poly_complex_workspace_alloc(len),
                                             [](gsl_poly_complex_workspace* pw){gsl_poly_complex_workspace_free(pw);});

    gsl_poly_complex_solve(coefficients.data(), len, w.get(), z.data());

    vector<complex<double>> roots; roots.resize(len-1);
    for (int i = 0; i < len-1; i++)
        roots.push_back({z[2*i],z[2*i+1]});
    return roots;
}

vector<double> combine_filters(vector<vector<t_complex>>& filters){
    if(filters.empty()) return vector<double>({1.0});

    vector<t_complex> h_c(filters.back()),prod;
    filters.pop_back();

    for(auto filter : filters){
        cout << filter.size() << endl;
        for(int i = 0; i < h_c.size(); i++){
            for(int j = 0; j < filter.size(); j++){
                if(i+j >= prod.size()) prod.push_back({0,0});
                prod.at(i+j) += h_c[i]*filter[j];
            }
        }
        h_c = prod;
        prod.clear();
    }

    vector<double> h;
    for(auto x : h_c){h.push_back(x.real());}
    return h;
}

vector<double> calc_transverse_filter(const vector<t_complex>& roots, double eps){
    t_complex mult{-1,0};

    vector<vector<t_complex>> filters;
    for(auto root : roots){
        if(abs(root) <= 1 ) continue;
        t_complex coef = conj(root)/pow(abs(root),2);
        mult *= coef;

        filters.push_back(vector<t_complex>{t_complex({1,0})});
        while(abs(coef) > eps){
            filters.back().push_back(coef);
            coef *= coef;
        }
    }

    vector<double> h = combine_filters(filters);
    for(auto &x : h){x *= mult.real();}
    return h;
}

vector<double> calc_recursive_filter(const vector<t_complex>& roots, double eps){

    vector<vector<t_complex>> filters;
    for(auto root : roots){
        if(abs(root) >= 1) continue;

        filters.push_back(vector<t_complex>{t_complex({1,0})});
        while(abs(root) > eps){
            filters.back().push_back(root);
            root *= root;
        }
    }

    return combine_filters(filters);
}

UncasualFilter invert(vector<double> h, double eps){
    UncasualFilter uf;

    vector<t_complex> roots = find_roots(h);
    uf.transverse = calc_transverse_filter(roots,eps);
    uf.recursive = calc_recursive_filter(roots,eps);

    reverse(uf.recursive.begin(),uf.recursive.end());

    return uf;
}

#endif //COMPPHYSFWK_UNCASUAL_HPP
