#include <iostream>
#include <cmath>
#include <algorithm>
#include <list>
#include <memory>

#include "../lib_fdtd/advanced/example_fdtd_2dite.hpp"
#include "../utils/faddeeva.h"
#include "../lib_fdtd/diplay/em_field_intensity_display.hpp"

using namespace std;

double GaussianPW(double lambda, double fi, double x, double y,  double x0, double y0){

	double kx = cos(fi); double ky = sin(fi);
	kx *= 2*M_PI/lambda; ky *= 2*M_PI/lambda;

	double w = lambda;

	using cmpl = complex<double>;
	cmpl A(1,0);
	A *= exp(cmpl(-(pow(x-x0,2) + pow(y-y0,2))/(w*w),0));
	if(A.real() < 1e-7) return 0;
	A *= exp(cmpl(0,kx*(x-x0)+ky*(y-y0)));

	//not working...
	//A *= (cmpl(1.0,0.0) + Faddeeva::erf(cmpl(kx*w/2.0,(x-x0)/w)));
	//A *= cmpl(1,0) + Faddeeva::erf(cmpl(ky*w/2,(y-y0)/w));

	return A.real();

}

#include "../lib_fdtd/advanced/example_fdtd_2nd_mur.hpp"

class ExLaserfdtd : public ExFDTD2ndTEMur {//Ex2DITEfdtd {
public:
	using tBase = ExFDTD2ndTEMur; //Ex2DITEfdtd;
	using tCell = tBase::tCell;
	using tInit = tBase::tInit;
	using tFCell = tBase::tFCell;

	ExLaserfdtd(tInit ti) : tBase(ti){
		tBase::sources.clear();

		double fi = M_PI/5;

		for(int i = 2; i < Nx-2; i++){
			for(int j = 2; j < Ny-2; j++){
				double x = i*dl;
				double y = j*dl;

				field[i][j].c.Hz = GaussianPW(lda_c, fi, x, y, dl*Nx/2, dl*Ny/2);

				field[i][j].c.Ex = -field[i][j].c.Hz*sin(fi)*2*M_PI/(w*lda_c);
				field[i][j].c.Ey = field[i][j].c.Hz*cos(fi)*2*M_PI/(w*lda_c);
			}
		}
	};

protected:
	tContainer &field = tBase::field;

private:
	const double lda_c = Ny*dl/(20);
	const double Vg = 1;
	const double w = 2*M_PI*Vg/(lda_c);

};

int main(int argc, char **argv){
	shared_ptr<ExLaserfdtd> field(new ExLaserfdtd(ExLaserfdtd::tInit{300,300}));

	Window w(unique_ptr<EMFieldIntensityDisplay<ExLaserfdtd>>(new EMFieldIntensityDisplay<ExLaserfdtd>(field)));
	w.create_window(600,600);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "Exit" << std::endl;

	return 0;
}
