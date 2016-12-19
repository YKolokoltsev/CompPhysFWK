/*
 * example_fdtd_2dite.hpp
 *
 *  Created on: Nov 6, 2016
 *      Author: morrigan
 */

#ifndef SRC_EXAMPLE_FDTD_2DITE_HPP_
#define SRC_EXAMPLE_FDTD_2DITE_HPP_

#include "lib_fdtd/lib_fdtd.h"
#include "lib_visual/lib_visual.h"

using namespace std;

class Ex2DITEfdtd : public EMLeapfrogMultiproc<Field2D, IsotropicMaterial<Cell2DTE<Plain2DIndex>>>{
public:
	using tBase = EMLeapfrogMultiproc<Field2D, IsotropicMaterial<Cell2DTE<Plain2DIndex>>>;
	using tSourceBase = Source<Plain2DIndex>;
	using tCell = tBase::tCell;
	using tInit = tBase::tInit;
	using tFCell = tBase::tFCell;

	Ex2DITEfdtd(tBase::tInit ti) : tBase(ti),
		Nx(field.getNx()), Ny(field.getNy()){

		double lda = 20*dl; // = Vg*T = 2pi/w
		double Vg = 1;
		double w = 2*M_PI*Vg/(lda);

		sources.resize(1,shared_ptr<tSourceBase>((tSourceBase*) new SinSource<Plain2DIndex>(w)));
		sources[0]->i = Nx/2;
		sources[0]->j = Ny/2;

		for(int j = 0; j < Ny; j++){
			for(int i = 0; i < Nx; i++){
				field[i][j].c.i = i;
				field[i][j].c.j = j;
			}
		}
	};

	const size_t& getNx(){return Nx;};
	const size_t& getNy(){return Ny;};
protected:
	//AREA

	virtual void evalE(tCell& c){
		const tCell& cij1 = field[c.i][c.j-1].c;
		const tCell& ci1j = field[c.i-1][c.j].c;

		c.Ex += (c.Hz/c.eps - cij1.Hz/cij1.eps)*dt/dl;
		c.Ey += -(c.Hz/c.eps - ci1j.Hz/ci1j.eps)*dt/dl;
	};

	virtual void evalH(tCell& c){
		const tCell& cij1 = field[c.i][c.j+1].c;
		const tCell& ci1j = field[c.i+1][c.j].c;

		c.Hz += ((cij1.Ex/cij1.mu - c.Ex/c.mu) - (ci1j.Ey/ci1j.mu - c.Ey/c.mu))*dt/dl;
	};

	virtual void postH(){
		for_each(sources.begin(), sources.end(),[this](shared_ptr<tSourceBase>& src){
			field[src->i][src->j].c.Hz += src->shine(dt);
		});
	};

protected:
	tContainer &field = tBase::container;
	const size_t &Nx;
	const size_t &Ny;

	vector<shared_ptr<tSourceBase>> sources;

	//dt/dl = numeric Vg - how much dt's shell pass for energy to move distance of one YeeCell
	const double dt = 0.2;
	const double dl = 1;
};


#endif /* SRC_EXAMPLE_FDTD_2DITE_HPP_ */
