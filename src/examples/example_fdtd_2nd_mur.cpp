/*
 * example_fdtd_2nd_mur.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include "../lib_fdtd/advanced/example_fdtd_2nd_mur.hpp"
#include "../lib_fdtd/diplay/em_field_intensity_display.hpp"

using namespace std;

int main(int argc, char **argv){

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
