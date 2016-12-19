/*
 * example_fdtd_2nd_mur.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: morrigan
 */

#include "example_fdtd_2nd_mur.hpp"

#include <iostream>
#include <memory>

#include "lib_visual/lib_visual.h"
#include "lib_fdtd/diplay/em_field_intensity_display.hpp"


using namespace std;

int main(){

	shared_ptr<ExFDTD2ndTEMur> field(new ExFDTD2ndTEMur(ExFDTD2ndTEMur::tInit{300,300}));

	Window w(unique_ptr<EMFieldIntensityDisplay<ExFDTD2ndTEMur>>(new EMFieldIntensityDisplay<ExFDTD2ndTEMur>(field)));
	w.create_window(500,500);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "Exit" << std::endl;

	return 0;
}
