/*
 * log_energy_density_display.hpp
 *
 *  Created on: Nov 5, 2016
 *      Author: morrigan
 */

#ifndef SRC_LIB_FDTD_DIPLAY_LOG_ENERGY_DENSITY_DISPLAY_HPP_
#define SRC_LIB_FDTD_DIPLAY_LOG_ENERGY_DENSITY_DISPLAY_HPP_

#include <algorithm>
#include <memory>

#include "../lib_fdtd.h"

using namespace std;

template<typename TField>
class LogEnergyDensityDisplay : public Bitmap2DRenderer{
public:
	using tBase = Bitmap2DRenderer;
	using tEMFieldPtr = shared_ptr<TField>;
	using tData = typename TField::tData;
	using tVec = typename TField::tCell::tVec;
	using tScale = struct{double a; double b;};

	LogEnergyDensityDisplay(tEMFieldPtr fp) : Bitmap2DRenderer(), field_proc{fp},
				wE({fp->getNx(),fp->getNy()}), wH({fp->getNx(),fp->getNy()}), w({fp->getNx(),fp->getNy()}),
				Nx(fp->getNx()), Ny(fp->getNy()){};

protected:
	void draw_memory(){
		update_density();

		for_all([&](int i, int j){
			unsigned char e = wE[i][j] < scale1.a ? 0 : 255*log10(wE[i][j]/scale1.a)/scale1.b;
			unsigned char h = wH[i][j] < scale2.a ? 0 : 255*log10(wH[i][j]/scale2.a)/scale2.b;

			//TODO: Temporary solution
			unsigned char s = w[i][j] < scale.a ? 0 : 255*log10(w[i][j]/scale.a)/scale.b;

			s = s !=0 ? 255-s : 0;
			al_put_pixel(i,j,al_map_rgb(s, s , s));
		});
	};

	size_t& getNx(){return Nx;};
	size_t& getNy(){return Ny;};

	void update_density(){
		tData data = field_proc->copy_data();

		scale1 = {0,0};
		scale2 = {0,0};

		for_all([&](int i, int j){
			tVec v = data[i][j].c.E();
			wE[i][j] = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
			if(wE[i][j] != 0){
				scale1.a += wE[i][j];
				if(wE[i][j] > scale1.b) scale1.b = wE[i][j];
			}

			v = data[i][j].c.H();
			wH[i][j] = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
			if(wH[i][j] != 0){
				scale2.a += wH[i][j];
				if(wH[i][j] > scale2.b) scale2.b = wH[i][j];
			}

			if(wH[i][j] != 0 || wE[i][j] != 0){
				w[i][j] = wH[i][j] + wE[i][j];
				scale.a += w[i][j];
				if(w[i][j] > scale.b) scale.b = w[i][j];
			}
		});


		if(scale1.a != 0 && scale2.a != 0){
			scale1.a = scale1.a/(100*Nx*Ny);
			scale1.b = log10(scale1.b/scale1.a);
			scale2.a = scale2.a/(100*Nx*Ny);
			scale2.b = log10(scale2.b/scale2.a);
			scale.a = scale.a/(100*Nx*Ny);
			scale.b = log10(scale.b/scale.a)+1;
		}else{
			scale1 = {1,2};
			scale2 = {1,2};
			scale = {1,2};
		}
	}

	inline void for_all(function<void(int,int)> f){
		for(int i = 0; i < Nx; i++)
			for(int j = 0; j < Ny; j++)	f(i,j);
	}

protected:
	Field2D<double> wE;
	Field2D<double> wH;
	Field2D<double> w;

	tScale scale1{1,2};
	tScale scale2{1,2};
	tScale scale{1,2};

	tEMFieldPtr field_proc;
	size_t Nx;
	size_t Ny;
};




#endif /* SRC_LIB_FDTD_DIPLAY_LOG_ENERGY_DENSITY_DISPLAY_HPP_ */
