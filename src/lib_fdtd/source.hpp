/*
 * source.hpp
 *
 *  Created on: Nov 4, 2016
 *      Author: morrigan
 */

#ifndef SRC_LIB_FDTD_SOURCE_HPP_
#define SRC_LIB_FDTD_SOURCE_HPP_

#include <cmath>

template<typename TLocalIndex>
class Source : public TLocalIndex{
public:
	virtual ~Source(){};
	virtual double shine(double dt) = 0;
private:
	double prev;
};

template<typename TLocalIndex>
class SinSource : public Source<TLocalIndex>{
public:
	SinSource(double w, double mag = 1, double phase = 0) :
	mag{mag}, phase{phase}, w{w}{};

	double shine(double dt){
		double s = mag*sin(w*t + phase);
		t += dt;
		if(w*t > 2*M_PI) t -= 2*M_PI/w;
		return s;
	}

	double mag = 1;
	double phase = 0;
	double w;
protected:
	double t = 0;
};


template<typename TLocalIndex>
class ExpSinPulseSource : public Source<TLocalIndex>{
public:
	ExpSinPulseSource(double w, int Nt, double phi = 0) : w{w}{
		tau = Nt*M_PI/(2*w);
		double intpart;
		phase = 2*M_PI*modf(phi/(2*M_PI),&intpart);
		t = -tau;
	};

	double shine(double dt){
		double s = exp(-pow((t-tau)/tau,2))*mag*sin(w*(t-tau) + phase);
		t += dt;
		return s;
	};

	double sinshine(double dt){
		double s = mag*sin(w*(t-tau) + phase);
		t += dt;
		return s;
	}

	bool is_dead(){return t > 3*tau;};

	double mag = 1;
	double w;
	double tau;

protected:
	double t = 0;
	double phase = 0;
};

//USEFUL FUNCTIONS

double GaussianMag(double lambda, double fi, double x, double y,  double x0, double y0, double w0){

	double ex = cos(fi); double ey = sin(fi);
	double r = abs((-ey*(x-x0)+ex*(y-y0)));

	double z = (ex*(x-x0) + ey*(y-y0));

	//double w0 = lambda; //decay constant
	double Zr = M_PI*w0*w0/lambda;
	double wz = w0*sqrt(1+pow(z/Zr,2));
	double Rz_1 = z/(z*z + Zr*Zr);
	double Psi = atan(z/Zr);

	double A = cos(r*r*M_PI*Rz_1/(lambda))*exp(-pow(r/wz,2))*w0/wz;
	return A*cos(-2*M_PI*z/lambda);
};

inline double sinc(double x){
	if(abs(x) < 0.1) return 1 - pow(x,2)/6.0 + pow(x,4)/120.0 - pow(x,6)/5040.0;
	return sin(x)/x;
}



#endif /* SRC_LIB_FDTD_SOURCE_HPP_ */
