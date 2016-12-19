/*
 * em_field_intensity_display.hpp
 *
 *  Created on: Nov 5, 2016
 *      Author: morrigan
 */

#ifndef SRC_LIB_FDTD_DIPLAY_EM_FIELD_INTENSITY_DISPLAY_HPP_
#define SRC_LIB_FDTD_DIPLAY_EM_FIELD_INTENSITY_DISPLAY_HPP_

#include "log_energy_density_display.hpp"

template<typename TField>
class EMFieldIntensityDisplay : public LogEnergyDensityDisplay<TField>{
public:
	using tBase = LogEnergyDensityDisplay<TField>;
	using tEMFieldPtr = typename tBase::tEMFieldPtr;
	using tScale = typename tBase::tScale;

	EMFieldIntensityDisplay(tEMFieldPtr fp) : tBase(fp){};

protected:
	void draw_memory(){
		update_density();

		tBase::for_all([&](int i, int j){
			unsigned char e = wE[i][j] > scale1.a ? 255 : 255*wE[i][j]/scale1.a;
			unsigned char h = wH[i][j] > scale1.b ? 255 : 255*wH[i][j]/scale1.b;

			al_put_pixel(i,j,al_map_rgb(0, e, h));
		});
	};

	void update_density(){
		typename tBase::tData data = tBase::field_proc->copy_data();

		scale1 = {1,1};
		int area = 0;

		tBase::for_all([&](int i, int j){
			typename tBase::tVec v = data[i][j].c.E();
			wE[i][j] = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

			v = data[i][j].c.H();
			wH[i][j] = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

			if(wE[i][j] != 0 || wH[i][j] != 0){
				scale1.a += wE[i][j];
				scale1.b += wH[i][j];
				area++;
			}
		});

		if(area != 0){
			scale1.a = 10*scale1.a/area;
			scale1.b = 10*scale1.b/area;
		}

	}

	Field2D<double> &wE = tBase::wE;
	Field2D<double> &wH = tBase::wE;

	tScale &scale1 = tBase::scale1;
};



#endif /* SRC_LIB_FDTD_DIPLAY_EM_FIELD_INTENSITY_DISPLAY_HPP_ */
