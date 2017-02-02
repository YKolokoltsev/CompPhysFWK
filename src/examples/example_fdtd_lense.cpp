/*
 * example_fdtd_2dte_lence.cpp
 *
 *  Created on: Nov 2, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory>

#include "../lib_fdtd/advanced/example_fdtd_2nd_mur.hpp"
#include "../lib_fdtd/diplay/em_field_intensity_display.hpp"

using namespace std;

class ExDifInt : public ExFDTD2ndTEMur {
public:
	using tBase = ExFDTD2ndTEMur;
	using tCell = tBase::tCell;
	using tSourceBase = typename tBase::tSourceBase;
	using tInit = tBase::tInit;

	ExDifInt(tInit ti) : tBase(ti){

		double lda = 20*dl; // = Vg*T = 2pi/w
		double Vg = 1;
		double w = 2*M_PI*Vg/(lda);
		sources.clear();
		sources.resize(Ny);

		for(int j = 0; j < Ny; j++){
			for(int i = 0; i < Nx; i++){
				if(opcs::is_pt_in_lp_lens(200,Ny/2,250,230,200,i,j)){
					field[i][j].c.eps = 3.7;
				}
			}
			double mag = GaussianMag(lda,0, 2*dl, j*dl, 2*dl, dl*Ny/2, lda);
			sources[j] = shared_ptr<tSourceBase>((tSourceBase*)new SinSource<Plain2DIndex>(w,mag));
			sources[j]->i = 2;
			sources[j]->j = j;
		}
	};

private:
	tContainer &field = tBase::container;
	vector<shared_ptr<tSourceBase>> &sources = tBase::sources;
};

int main(int argc, char **argv){

	shared_ptr<ExDifInt> field(new ExDifInt(ExDifInt::tInit{600,200}));

	Window w(unique_ptr<EMFieldIntensityDisplay<ExDifInt>>(new EMFieldIntensityDisplay<ExDifInt>(field)));
	w.create_window(600,200);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "exit" << std::endl;

	return 0;
}




