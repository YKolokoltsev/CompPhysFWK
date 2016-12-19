/*
 * yee_cell.hpp
 *
 *  Created on: Nov 2, 2016
 *      Author: morrigan
 */

#ifndef SRC_LIB_FDTD_YEE_CELL_HPP_
#define SRC_LIB_FDTD_YEE_CELL_HPP_

#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>

using namespace std;

/*INDEXES*/
struct EmptyIndex{};

class Plain2DIndex{
public:
	Plain2DIndex(){};
	Plain2DIndex(initializer_list<size_t> c) {
		i = *c.begin();
		j = *(c.begin()+1);

	}

	size_t i = 0;
	size_t j = 0;

	bool operator< (const Plain2DIndex& x) const
	{ if(i != x.i) return i < x.i; return j < x.j; }
};

struct Ptr2DIndex{
	vector<Ptr2DIndex*> neighbors;
	double x = 0;
	double y = 0;
};

/*BASIC YEE CELLS*/
template <typename TLocalIndex>
class Cell2DTE : public TLocalIndex{
public:
	using tLocalIndex = TLocalIndex;
	using tVec = vector<double>;

	double Ex = 0;
	double Ey = 0;
	double Hz = 0;

	tVec E(){return tVec{Ex,Ey,0};}
	tVec H(){return tVec{0,0,Hz};}
};

template <typename TLocalIndex>
class Cell2DTM : public TLocalIndex{
public:
	using tLocalIndex = TLocalIndex;
	using tVec = vector<double>;

	double Hx = 0;
	double Hy = 0;
	double Ez = 0;

	tVec E(){return tVec{0,0,Ez};}
	tVec H(){return tVec{Hx,Hy,0};}
};

/*ADVANCED YEE CELLS*/
template <typename TLocalIndex>
class Cell2D : public Cell2DTE<TLocalIndex>, public Cell2DTM<EmptyIndex>{
public:
	using tTE = Cell2DTE<TLocalIndex>;
	using tTM = Cell2DTM<EmptyIndex>;
	using tLocalIndex = typename tTE::tLocalIndex;
	using tVec = typename tTE::tVec;

	tVec E(){return tVec{tTE::Ex,tTE::Ey,tTM::Ez};}
	tVec H(){return tVec{tTM::Hx,tTM::Hy,tTE::Hz};}
};

template <typename TCell>
class IsotropicMaterial : public TCell{
public:
	double eps = 1;
	double mu  = 1;
};

template <typename TLocalIndex>
class Cell2DTEMatch : public IsotropicMaterial<Cell2DTE<TLocalIndex>> {
public:
	using tBase = IsotropicMaterial<Cell2DTE<TLocalIndex>>;
	using tVec = typename tBase::tVec;

	double sigEx = 0;
	double sigEy = 0;

	double sigHzx = 0;
	double sigHzy = 0;

	double Hzx = 0;
	double Hzy = 0;

	tVec H(){
		tVec H = tBase::H();
		if(H[2] == 0) H[2] = Hzx + Hzy;
		return H;
	}
};

template <typename TLocalIndex>
class Cell2DTMMatch : public IsotropicMaterial<Cell2DTM<TLocalIndex>> {
public:
	using tBase = IsotropicMaterial<Cell2DTM<TLocalIndex>>;
	using tVec = typename tBase::tVec;

	double sigEzx = 0;
	double sigEzy = 0;

	double sigHx = 0;
	double sigHy = 0;

	double Ezx = 0;
	double Ezy = 0;

	tVec E(){
		tVec E = tBase::E();
		if(E[2] == 0) E[2] = Ezx + Ezy;
		return E;
	}
};

template <typename TLocalIndex>
class Cell2DMatch : public Cell2DTEMatch<TLocalIndex>, public Cell2DTMMatch<EmptyIndex>{
public:
	using tTE = Cell2DTEMatch<TLocalIndex>;
	using tTM = Cell2DTMMatch<EmptyIndex>;
	using tVec = typename tTE::tVec;

	tVec E(){
		tVec Ete = tTE::E();
		tVec Etm = tTM::E();
		Ete[0] += Etm[0];
		Ete[1] += Etm[1];
		Ete[2] += Etm[2];
		return Ete;
	}

	tVec H(){
		tVec Hte = tTE::H();
		tVec Htm = tTM::H();
		Hte[0] += Htm[0];
		Hte[1] += Htm[1];
		Hte[2] += Htm[2];
		return Hte;
	}
};

#endif /* SRC_LIB_FDTD_YEE_CELL_HPP_ */
