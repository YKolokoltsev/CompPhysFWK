/*
 * test_concept.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: morrigan
 */

#include <type_traits>
#include <iostream>
#include <typeinfo>
#include <complex>
#include <memory>
#include <cmath>
#include <algorithm>

#include <gsl/gsl_poly.h>

#include "utils/faddeeva.h"
#include "lib_visual/gnuplot.hpp"

using namespace std;



/////////////////////////////////////
using t_complex = complex<double>;

vector<complex<double>> find_roots(const vector<double>& coefs){
	vector<double> z; z.resize((coefs.size()-1)*2);

	using t_ws = gsl_poly_complex_workspace;
	shared_ptr<t_ws> w(gsl_poly_complex_workspace_alloc (coefs.size()),
			[=](t_ws* w){gsl_poly_complex_workspace_free(w);});

	gsl_poly_complex_solve(coefs.data(), coefs.size(), w.get(), z.data());

	vector<complex<double>> roots;
	for (int i = 0; i < coefs.size()-1; i++)
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

vector<double> convolve(const vector<double>& h, const vector<double>& x){
	vector<double> y;
	y.resize(x.size() + h.size());
	for(int k = 0; k < x.size(); k++)
		for(int m = k; m < h.size() + k; m++){
			y.at(m) += x.at(k)*h.at(m-k);
		}
	return y;
}

vector<double> deconvolve(const vector<double>& h, const vector<double>& x){
	vector<double> y;
	vector<double> b;
	for(int i = 1; i < h.size(); i++) b.push_back(-h[i]);

	for(int k = 0; k < x.size(); k++){

	//	for(int m = 0; m < min(1.0*y.size()-1,1.0*b.size()); m++)

	}

	return y;
}

int main(){

	vector<double> coefs{1,2,3,4};
	vector<t_complex> roots = find_roots(coefs);
	auto h_trans = calc_transverse_filter(roots,0.001);
	auto h_recur = calc_recursive_filter(roots,0.001);


	reverse(h_recur.begin(),h_recur.end());
	h_recur.insert( h_recur.end(), h_trans.begin(), h_trans.end() );
	//reverse(h_recur.begin(),h_recur.end());
	vector<double> restored = convolve(coefs, h_trans);

	Plotter plotter("set term x11 enhanced font 'arial,15' persist");
	vector<pair<double,double>> data;

	for(int i = 0; i < coefs.size();i++) data.push_back({i,coefs[i]});
	plotter.append(data, "with lines title 'Coefs'");
	data.clear();

	for(int i = 0; i < h_recur.size();i++) data.push_back({i,h_recur[i]});
	plotter.append(data, "with lines title 'Recur'");
	data.clear();

	for(int i = 0; i < restored.size();i++) data.push_back({i,restored[i]});
	plotter.append(data, "with lines title 'Restored'");
	data.clear();

	plotter.show();

	return 0;
}
