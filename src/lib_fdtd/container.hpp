/*
 * emfield.h
 *
 *  Created on: Jan 26, 2016
 *      Author: morrigan
 */

#ifndef CONTAINER_HPP_
#define CONTAINER_HPP_

#include <vector>
#include <iostream>
#include <functional>


using namespace std;

template <typename TFCell>
class FieldIterator{
public:
	using iterator = typename vector<TFCell*>::iterator;
	using const_iterator = typename vector<TFCell*>::const_iterator;

	void for_bounds(function<void(TFCell*)> f){for_each(bounds.begin(), bounds.end(), f);}
	void for_body(function<void(TFCell*)> f){for_each(body.begin(), body.end(), f);}
	void for_all(function<void(TFCell*)> f){for_each(all.begin(), all.end(), f);}

protected:
	vector<TFCell*> bounds;
	vector<TFCell*> body;
	vector<TFCell*> all;
private:
	inline void for_each(iterator start, const_iterator end, function<void(TFCell*)> f){
		while(start < end){
			f(*start);
			start++;
		}
	}
};

template <typename TFCell>
class Field1D : public FieldIterator<TFCell>{
public:
	using tInit = size_t;
	using tBase = FieldIterator<TFCell>;
	using tData = vector<TFCell>;

	Field1D(tInit ti) : N{ti}{
		data.resize(N);

		size_t i = 0;
		for(auto& el : data){
			if(i == 0 || i == N-1){
				tBase::bounds.push_back(&el);
			}else{
				tBase::body.push_back(&el);
			}
			tBase::all.push_back(&el);
			i++;
		}
	};

	TFCell& operator[](std::size_t idx){ return data[idx]; }
	size_t& getN(){return N;}

public:
	tData data;

private:
	template<template<typename> class TContainer, typename TCell>
	friend class EMLeapfrog;
	tData copy_data(){return copy(data);};

private:
	size_t N;
};

template <typename TFCell>
class Field2D : public FieldIterator<TFCell>{
public:
	using tInit = struct {size_t Nx; size_t Ny;};
	using tBase = FieldIterator<TFCell>;
	using tData = vector<vector<TFCell>>;

	Field2D(tInit ti): Nx{ti.Nx}, Ny{ti.Ny}{
		data.resize(Nx);
		for(auto& line : data) line.resize(Ny);
		TFCell* pel;
		for(size_t i = 0; i < Nx; i++)
			for(size_t j = 0; j < Ny; j++){
				pel = &data[i][j];
				if(i == 0 || i == (Nx-1) || j == 0 || j == (Ny-1)){
					tBase::bounds.push_back(pel);
				}else{
					tBase::body.push_back(pel);
				}
				tBase::all.push_back(pel);
			}
	};

	vector<TFCell>& operator[](std::size_t idx){ return data[idx]; }
	const size_t& getNx(){return Nx;}
	const size_t& getNy(){return Ny;}

public:
	tData data;
	size_t Nx;
	size_t Ny;

private:
	template<template<typename> class TContainer, typename TCell>
	friend class EMLeapfrog;
	tData copy_data(){
		tData copy;
		copy.resize(Nx);
		for(int i = 0; i < Nx; i++)copy[i] = data[i];
		return copy;
	};

};

#endif /* CONTAINER_HPP_ */
