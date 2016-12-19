/*
 * example_fdtd_2dte_lence.cpp
 *
 *  Created on: Nov 2, 2016
 *      Author: morrigan
 */

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>

#include "example_fdtd_2nd_mur.hpp"

#include "lib_fdtd/lib_fdtd.h"
#include "lib_visual/lib_visual.h"
#include "lib_fdtd/diplay/em_field_intensity_display.hpp"
#include "lib_fdtd/diplay/log_energy_density_display.hpp"

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

		int Cy1 = Ny/2.0 + Ny/8.0;
		int Cy2 = Ny/2.0 - Ny/8.0;

		for(int j = 0; j < Ny; j++){
			for(int i = 0; i < Nx; i++){
				if(i >= Nx/5 && i <= (Nx/5+1)){
					if(!((j >= Cy1-2 && j <= Cy1+2) || (j >= Cy2-2 && j <= Cy2+2)) && (j >= 3 && j <=Ny-3))
					field[i][j].c.eps = 100;
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

int main(){

	shared_ptr<ExDifInt> field(new ExDifInt(ExDifInt::tInit{400,200}));

	Window w(unique_ptr<LogEnergyDensityDisplay<ExDifInt>>(new LogEnergyDensityDisplay<ExDifInt>(field)));
	w.create_window(600,300);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "exit" << std::endl;

	return 0;
}




