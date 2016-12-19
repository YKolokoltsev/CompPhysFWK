/*
 * optics.h
 *
 *  Created on: Jan 29, 2016
 *      Author: morrigan
 */

#ifndef SRC_FDTD_OPTICS_H_
#define SRC_FDTD_OPTICS_H_

#include <cmath>

using namespace std;

namespace opcs{

bool is_pt_in_pp_plate(double cx, double cy, double width, double height, double pt_x, double pt_y){
	return ((std::abs(pt_x - cx) <= width/2) && (std::abs(pt_y - cy) <= height/2));
}

bool is_pt_in_lp_lens(double cx, double cy, double r, double h, double H, double pt_x, double pt_y){
	pt_x -= cx+h;
	pt_y -= cy;

	if(pt_x > -h) return false;
	if(abs(pt_y) > H/2) return false;

	if(sqrt(pt_x*pt_x + pt_y*pt_y) > r) return false;

	return true;
}

inline double sinc(double x){
	if(abs(x) < 0.1) return 1 - pow(x,2)/6.0 + pow(x,4)/120.0 - pow(x,6)/5040.0;
	return sin(x)/x;
}

}



#endif /* SRC_FDTD_OPTICS_H_ */
