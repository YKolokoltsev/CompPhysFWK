/*
 * example_fdtd_laser.cpp
 *
 *  Created on: Nov 6, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <iostream>
#include <cmath>
#include <algorithm>
#include <list>
#include <memory>

#include "../../lib_fdtd/advanced/example_fdtd_2dite.hpp"
#include "../../lib_fdtd/advanced/example_fdtd_2nd_mur.hpp"
#include "../../lib_fdtd/diplay/em_field_intensity_display.hpp"

using namespace std;

class ExcitedAtom : public Source<Plain2DIndex>{
public:
	using tBase = Source<Plain2DIndex>;
	using tSinPulse = ExpSinPulseSource<EmptyIndex>;

	ExcitedAtom(double w, bool is_spontaneous, double dt, size_t i, size_t j) : w{abs(w)}, dt{dt}{
		tBase::i = i; tBase::j = j;
		double phase = 0;
		if(is_spontaneous) src.reset(new tSinPulse(w,Nt,phase));
		else sync_cycles = (2*M_PI/w)/dt;
	};
	double shine(double dt){
		if(src) return src->shine(dt);
		return 0;
	}
	void sync(double x){
		fieldlog.push_back(x); sync_cycles--;
		if(sync_cycles < 0){

			double phi_max = 0;
			double prod_max = 0;

			for(double phi = 0; phi < 2*M_PI; phi += 0.01){
				double prod = 0;
				tSinPulse y(w,Nt,phi);

				for(auto x : fieldlog) prod += x*y.sinshine(dt);

				if(prod > prod_max){
					phi_max = phi;
					prod_max = prod;
				}
			}

			src.reset(new tSinPulse(w,Nt,phi_max));
		}
	}
	bool is_active(){return src != nullptr;}
	bool is_dead(){
		if(!is_active()) return false;
		return src->is_dead();
	}
private:
	const int Nt = 5;
	const double dt;
	int sync_cycles = 0;
	vector<double> fieldlog;
	shared_ptr<ExpSinPulseSource<EmptyIndex>> src;
	double w;
};

class ExLaserfdtd : public Ex2DITEfdtd {
public:
	using tBase = Ex2DITEfdtd;
	using tCell = tBase::tCell;
	using tInit = tBase::tInit;
	using tFCell = tBase::tFCell;

	ExLaserfdtd(tInit ti) : tBase(ti){ tBase::sources.clear(); };

protected:
	void postE(){
		update_sources();
		for(auto &src : atoms)
			if(src.is_active()){
				field[src.i][src.j].c.Hz += src.shine(dt);
			}else{
				src.sync(field[src.i][src.j].c.Hz);
			};
	}

private:
	void update_sources(){
		atoms.remove_if([=](ExcitedAtom& src)->bool{return src.is_dead();});

		size_t i = rand()%(Nx-3) + 1;
		size_t j = rand()%(Ny-3) + 1;

		atoms.push_back(ExcitedAtom(w, false, dt,i,j));
	}

protected:
	tContainer &field = tBase::field;
	list<ExcitedAtom> atoms;

private:
	const double lda_c = Ny*dl/3;
	const double Vg = 1;
	const double w = 2*M_PI*Vg/(lda_c);
};

int main(int argc, char **argv){
	shared_ptr<ExLaserfdtd> field(new ExLaserfdtd(ExLaserfdtd::tInit{200,200}));

	Window w(unique_ptr<EMFieldIntensityDisplay<ExLaserfdtd>>(new EMFieldIntensityDisplay<ExLaserfdtd>(field)));
	w.create_window(600,600);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "Exit" << std::endl;

	return 0;
}

