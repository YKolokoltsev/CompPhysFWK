/*
 * example_fdtd_laser.cpp
 *
 *  Created on: Nov 6, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <vector>
#include <memory>

#include "../lib_fdtd/advanced/example_fdtd_2dite.hpp"
#include "../lib_fdtd/diplay/em_field_intensity_display.hpp"

using namespace std;

int main(int argc, char **argv){
	shared_ptr<Ex2DITEfdtd> field(new Ex2DITEfdtd(Ex2DITEfdtd::tInit{400,300}));

	Window w(unique_ptr<EMFieldIntensityDisplay<Ex2DITEfdtd>>(new EMFieldIntensityDisplay<Ex2DITEfdtd>(field)));
	w.create_window(600,600);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "Exit" << std::endl;

	return 0;
}


