/*
 * leapfrog.h
 *
 *  Created on: Oct 25, 2016
 *      Author: morrigan
 */

#ifndef SRC_LEAPFROG_HPP_
#define SRC_LEAPFROG_HPP_

#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

using namespace std;

template<template<typename> class TContainer, typename TCell>
class EMLeapfrog {

public:
	using tFPtr = void (EMLeapfrog::*) (TCell&);
	using tCell = TCell;
	using tFCell = struct {tFPtr evalE; tFPtr evalH; TCell c;};
	using tContainer = TContainer<tFCell>;
	using tInit = typename tContainer::tInit;
	using tData = typename tContainer::tData;

	EMLeapfrog(tInit ti): container(ti), lock_count{0}
	{
		container.for_body([](tFCell* fcell){
			fcell->evalE = &EMLeapfrog::evalE;
			fcell->evalH = &EMLeapfrog::evalH;
		});

		container.for_bounds([](tFCell* fcell){
			fcell->evalE = &EMLeapfrog::evalB;
			fcell->evalH = &EMLeapfrog::evalB;
		});
	};

	virtual ~EMLeapfrog(){};

	virtual void evaluate(){
		while(lock_count > 0) this_thread::sleep_for(std::chrono::milliseconds(1));
		mtx.lock();
		container.for_all([this](tFCell* fcell){(this->*fcell->evalE)(fcell->c);});
        postE();
        container.for_all([this](tFCell* fcell){(this->*fcell->evalH)(fcell->c);});
		postH();
		mtx.unlock();
	};

	tData copy_data(){
		lock_count++;
		mtx.lock();
		lock_count--;
		tData data = container.copy_data();
		mtx.unlock();
		return data;
	};

protected:

	virtual void evalE(TCell&){};
	virtual void evalH(TCell&){};
	virtual void postE(){};
	virtual void postH(){};

	tContainer container; //EM field

private:
	void evalB(TCell&){};

	template<template<typename> class A, typename B>
	friend class EMLeapfrogMultiproc;

	atomic_ulong lock_count;
	mutex mtx;
};

template<template<typename> class TContainer, typename TCell>
class EMLeapfrogMultiproc : public EMLeapfrog<TContainer,TCell>{
public:

	using tBase = EMLeapfrog<TContainer,TCell>;
	using tFPtr = typename tBase::tFPtr;
	using tCell = TCell;
	using tFCell = typename tBase::tFCell;
	using tContainer = typename tBase::tContainer;
	using tInit = typename tBase::tInit;
	using tData = typename tContainer::tData;

	EMLeapfrogMultiproc(tInit ti) : tBase(ti) {
		ncores = std::thread::hardware_concurrency();
		if(!ncores) ncores = 1;
		cout << "Using " << ncores << " cores" << endl;

		threads.resize(ncores);
		thread_cells.resize(ncores);

		container.for_all([this](tFCell* fcell){
			thread_cells[rand() % this->ncores].push_back(fcell);
		});
	};

	virtual ~EMLeapfrogMultiproc(){
		for(auto& x : threads) if(x.joinable()) x.join();
	};

	//TODO: Recreate threads is too slow, better synchronize them
	virtual void evaluate(){
		while(tBase::lock_count > 0) this_thread::sleep_for(std::chrono::milliseconds(1));

		tBase::mtx.lock();
		for(int i = 0; i < ncores; i++)
			threads[i] = thread(thread_eval_E,this,i);
		for(auto& x : threads) x.join();

		postE();

		for(int i = 0; i < ncores; i++)
			threads[i] = thread(thread_eval_H,this,i);
		for(auto& x : threads) x.join();

		postH();
		tBase::mtx.unlock();
	};

	tData copy_data(){return tBase::copy_data();};

protected:
	virtual void postE(){};
	virtual void postH(){};

	tContainer &container = tBase::container;

private:

	static void thread_eval_E(EMLeapfrogMultiproc<TContainer,TCell>* context, int target_thread){
		for(auto p_cell : context->thread_cells[target_thread])
			(context->*p_cell->evalE)(p_cell->c);
	};

	static void thread_eval_H(EMLeapfrogMultiproc<TContainer,TCell>* context, int target_thread){
		for(auto p_cell : context->thread_cells[target_thread])
					(context->*p_cell->evalH)(p_cell->c);
	};

	vector<vector<tFCell*>> thread_cells;
	vector<thread> threads;
	unsigned int ncores;
};

#endif /* SRC_LEAPFROG_HPP_ */
