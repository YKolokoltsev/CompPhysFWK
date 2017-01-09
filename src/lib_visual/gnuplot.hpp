/*
 * gnuplot.h
 *
 *  Created on: Jul 20, 2016
 *      Author: morrigan
 */

#ifndef SRC_GNUPLOT_HPP_
#define SRC_GNUPLOT_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>

using namespace std;

class Plotter{
public:
	/*
	 * Common terminals:
	 * 1. set term x11 enhanced font 'arial,15' persist
	 *
	 */

	Plotter(string terminal) : terminal(terminal){};

	void append(const vector<double>& data_y, string with){
		size_t len = data_y.size();
		Plot plot{{},with}; plot.data.resize(len);

		for(size_t i = 0; i < len; i++)
			plot.data[i] = make_pair(i, data_y[i]);

		plots.push_back(move(plot));
	}

	void append(const vector<pair<double, double>>& data, string with){
			plots.push_back(Plot{data,with});
		}

	void show(){
		if(plots.empty()) cerr << "Plotter: nothing to show" << endl;

			//find ranges
			int i = 0;
			pair<double,  double> xrange, yrange;

			ofstream data("data.txt", ofstream::out);
			for(auto& plot : plots){
				if(i > 0) data << endl << endl;
				for(auto& pt : plot.data){
					if(xrange.first > pt.first || i==0) xrange.first = pt.first;
					if(xrange.second < pt.first || i==0) xrange.second = pt.first;
					if(yrange.first > pt.second || i==0) yrange.first = pt.second;
					if(yrange.second < pt.second || i==0) yrange.second = pt.second;
					data << pt.first << " " << pt.second << endl;
					i++;
				}
			}
			data.close();

			ofstream script("plot.plt", ofstream::out);

			script << terminal << endl;
			script << "set xrange [" << xrange.first << ":" << xrange.second << "];" << endl;
			script << "set yrange [" << yrange.first << ":" << yrange.second << "];" << endl;

			i = 0;
			for(auto& plot : plots){
				if(i==0){
					script << "plot 'data.txt' index " << i << " using 1:2 " + plot.with;
				}else{
					script << ",\\" << endl << " \t'data.txt' index " << i << " using 1:2 " + plot.with;
				}
				i++;
			}
			script << endl;
			script.close();

			system("gnuplot plot.plt && rm plot.plt && rm data.txt");
	};

	void clear(){plots.clear();}

private:
	struct Plot{
		vector<pair<double, double>> data;
		string with;
	};

	string terminal;
	vector<Plot> plots;
};

#endif /* SRC_GNUPLOT_HPP_ */
