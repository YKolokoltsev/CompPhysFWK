/*
 * example_fdtd_2nd_mur.hpp
 *
 *  Created on: Nov 6, 2016
 *      Author: morrigan
 */

#ifndef SRC_EXAMPLE_FDTD_2ND_MUR_HPP_
#define SRC_EXAMPLE_FDTD_2ND_MUR_HPP_

#include <map>
#include <utility>

#include "lib_fdtd/lib_fdtd.h"
#include "example_fdtd_2dite.hpp"

using namespace std;

class ExFDTD2ndTEMur : public Ex2DITEfdtd {
public:
	using tBase = Ex2DITEfdtd;
	using tCell = tBase::tCell;
	using tSourceBase = typename tBase::tSourceBase;
	using tFPtr = tBase::tFPtr;
	using tInit = tBase::tInit;

	ExFDTD2ndTEMur(tInit ti) : tBase(ti){

		double lda = 20*dl; // = Vg*T = 2pi/w
		double Vg = 1;
		double w = 2*M_PI*Vg/(lda);

		sources.clear();
		sources.push_back(shared_ptr<tSourceBase>((tSourceBase*)new SinSource<Plain2DIndex>(w)));
		sources[0]->i = Nx/2;
		sources[0]->j = Ny/2;

		sources.push_back(shared_ptr<tSourceBase>((tSourceBase*)new SinSource<Plain2DIndex>(0.8*w)));
		sources[1]->i = sources[0]->i+1;
		sources[1]->j = sources[0]->j+1;

		field.for_bounds([this](tFCell* fcell){
			auto i = fcell->c.i;
			auto j = fcell->c.j;

			if(i != 0 && j != 0){
				fcell->evalE = (tFPtr)&ExFDTD2ndTEMur::evalE;
				if(j==Ny-1) fcell->evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBb;
				if(i==Nx-1)	fcell->evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBr;
			}
			if(i != Nx-1 && j != Ny-1){
				fcell->evalH = (tFPtr)&ExFDTD2ndTEMur::evalH;
				if(i == 0) fcell->evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBl;
				if(j == 0) fcell->evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBt;
			}
		});

		field[0   ][0   ].evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBtl;
		field[Nx-1][0   ].evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBtr;
		field[0   ][Ny-1].evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBbl;
		field[Nx-1][Ny-1].evalH = (tFPtr)&ExFDTD2ndTEMur::evalHBbr;

		update_b1();
		b2 = b1;
	};

protected:

	virtual void postE(){
		b2.clear();
		b2 = b1;
		update_b1();
	}

	tContainer &field = tBase::field;
	vector<shared_ptr<tSourceBase>> &sources = tBase::sources;

private:

	//EDGES
	void evalHBl(tCell& c){
		Neighbors ng1(b1,c,[](int di, int dj)->bool{return di != -1;});
		Neighbors ng2(b2,c,[](int di, int dj)->bool{return dj == 0 && di != -1;});

		tCell b0_r  = *ng1.pc(1,0);
		evalH(b0_r);

		c.Hz = -ng2.pc(1,0)->Hz + coe*(b0_r.Hz + ng2.pc(0,0)->Hz) + coe2*(c.Hz + ng1.pc(1,0)->Hz) +
				coe3*(ng1.pc(0,1)->Hz - 2*c.Hz + ng1.pc(0,-1)->Hz + ng1.pc(1,1)->Hz - 2*ng1.pc(1,0)->Hz + ng1.pc(1,-1)->Hz);
	};

	void evalHBt(tCell& c){
		Neighbors ng1(b1,c,[](int di, int dj)->bool{return dj != -1;});
		Neighbors ng2(b2,c,[](int di, int dj)->bool{return dj != -1 && di == 0;});

		tCell b0_b  = *ng1.pc(0,1);
		evalH(b0_b);

		c.Hz = -ng2.pc(0,1)->Hz + coe*(b0_b.Hz + ng2.pc(0,0)->Hz) + coe2*(c.Hz + ng1.pc(0,1)->Hz) +
				coe3*(ng1.pc(1,0)->Hz - 2*c.Hz + ng1.pc(-1,0)->Hz + ng1.pc(1,1)->Hz - 2*ng1.pc(0,1)->Hz + ng1.pc(-1,1)->Hz);
	};

	void evalHBr(tCell& c){
		Neighbors ng1(b1,c,[](int di, int dj)->bool{return di != 1;});
		Neighbors ng2(b2,c,[](int di, int dj)->bool{return dj == 0 && di != 1;});

		tCell b0_l = *ng1.pc(-1,0);
		evalH(b0_l);

		c.Hz = -ng2.pc(-1,0)->Hz + coe*(b0_l.Hz + ng2.pc(0,0)->Hz) + coe2*(c.Hz + ng1.pc(-1,0)->Hz) +
				coe3*(ng1.pc(0,1)->Hz - 2*c.Hz + ng1.pc(0,-1)->Hz + ng1.pc(-1,1)->Hz - 2*ng1.pc(-1,0)->Hz + ng1.pc(-1,-1)->Hz);
	};


	void evalHBb(tCell& c){
		Neighbors ng1(b1,c,[](int di, int dj)->bool{return dj != 1;});
		Neighbors ng2(b2,c,[](int di, int dj)->bool{return dj != 1 && di == 0;});

		tCell b0_t = *ng1.pc(0,-1);
		evalH(b0_t);

		c.Hz = -ng2.pc(0,-1)->Hz + coe*(b0_t.Hz + ng2.pc(0,0)->Hz) + coe2*(c.Hz + ng1.pc(0,-1)->Hz) +
				coe3*(ng1.pc(1,0)->Hz - 2*c.Hz + ng1.pc(-1,0)->Hz + ng1.pc(1,-1)->Hz - 2*ng1.pc(0,-1)->Hz + ng1.pc(-1,-1)->Hz);
	};

	//CORNERS
	void evalHBtl(tCell& c){
		tCell b1_br = b1[Plain2DIndex{1,1}];
		tCell b0_br = b1_br;
		evalH(b0_br);
		c.Hz = b1_br.Hz + coe_cor*(b0_br.Hz - c.Hz);
	};

	void evalHBtr(tCell& c){
		tCell b1_bl = b1[Plain2DIndex{Nx-2,1}];
		tCell b0_bl = b1_bl;
		evalH(b0_bl);
		c.Hz = b1_bl.Hz + coe_cor*(b0_bl.Hz - c.Hz);
	};

	void evalHBbl(tCell& c){
		tCell b1_tr = b1[Plain2DIndex{1,Ny-2}];
		tCell b0_tr = b1_tr;
		evalH(b0_tr);
		c.Hz = b1_tr.Hz + coe_cor*(b0_tr.Hz - c.Hz);
	};

	void evalHBbr(tCell& c){
		tCell b1_tl = b1[Plain2DIndex{Nx-2,Ny-2}];
		tCell b0_tl = b1_tl;
		evalH(b0_tl);
		c.Hz = b1_tl.Hz + coe_cor*(b0_tl.Hz - c.Hz);
	};

	void update_b1(){
		b1.clear();
		field.for_bounds([this](tFCell* fcell){
			auto i = fcell->c.i;
			auto j = fcell->c.j;

			b1.insert(make_pair(Plain2DIndex{i,j},field[i][j].c));

			if(i == 0) b1.insert(make_pair(Plain2DIndex{i+1,j},field[i+1][j].c));
			if(i == Nx-1) b1.insert(make_pair(Plain2DIndex{i-1,j},field[i-1][j].c));
			if(j == 0) b1.insert(make_pair(Plain2DIndex{i,j+1},field[i][j+1].c));
			if(j == Ny-1) b1.insert(make_pair(Plain2DIndex{i,j-1},field[i][j-1].c));
		});
	}

private:

	//mur
	map<Plain2DIndex,tCell> b1, b2;
	const double vp=1;
	const double coe = (dt - dl) / (dt + dl);
	const double coe_cor = (vp*dt - sqrt(2)*dl) / (vp*dt + sqrt(2)*dl);
	const double coe2 = (2*dl) / (vp*dt + dl);
	const double coe3 = (vp*dt)*(vp*dt) / (2*dl*(vp*dt + dl));

	class Neighbors{
	public:
		Neighbors(map<Plain2DIndex,tCell>& src, tCell& cent, function<bool(int di, int dj)> is_valid){
			for(int di = -1; di <= 1; di++)
				for(int dj = -1; dj <= 1; dj++)
					if(is_valid(di,dj)) mat[di+1][dj+1] = &src[Plain2DIndex{cent.i+di,cent.j+dj}];
		};
		inline const tCell* pc(int di, int dj){
			return mat[di+1][dj+1];
		};
	private:
		tCell* mat[3][3];
	};
};



#endif /* SRC_EXAMPLE_FDTD_2ND_MUR_HPP_ */
