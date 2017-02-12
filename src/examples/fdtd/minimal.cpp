/*
 * example_te2d.hpp
 *
 *  Created on: Oct 31, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#include <iostream>

#include "../../lib_fdtd/lib_fdtd.h"
#include "../../lib_visual/lib_visual.h"

class ExMin2Dfdtd : public EMLeapfrogMultiproc<Field2D, Cell2DTE<Plain2DIndex>>{
public:
	using tBase = EMLeapfrogMultiproc<Field2D, Cell2DTE<Plain2DIndex>>;
	using tCell = tBase::tCell;
	using tFPtr = tBase::tFPtr;

	ExMin2Dfdtd(tBase::tInit ti) : tBase(ti),
		Nx(field.getNx()), Ny(field.getNy()){

		field.for_bounds([](tFCell* fcell){
			fcell->evalE = (tFPtr)&ExMin2Dfdtd::evalB;
			fcell->evalH = (tFPtr)&ExMin2Dfdtd::evalB;
		});

		for(int i = 0; i < Nx; i++)
			for(int j = 0; j < Ny; j++){
				field[i][j].c.i = i;
				field[i][j].c.j = j;
				field[i][j].c.Ex = i;
			}
	};

	const size_t& getNx(){return Nx;};
	const size_t& getNy(){return Ny;};

private:
	void evalE(tCell& c){cout << "evalE [" << c.i << "," << c.j << "]" << endl;};
	void evalH(tCell& c){cout << "evalH [" << c.i << "," << c.j << "]" << endl;};
	void evalB(tCell& c){cout << "evalB [" << c.i << "," << c.j << "]" << endl;};
	void postE(){cout << "postE" << endl;};
	void postH(){cout << "postH" << endl;};

private:
	tContainer &field = tBase::container;
	const size_t &Nx;
	const size_t &Ny;
};

class ExMinTE2DRenderer : public Bitmap2DRenderer{
public:
	using tBase = Bitmap2DRenderer;
	using tEMFieldPtr = shared_ptr<ExMin2Dfdtd>;
	using tFont = unique_ptr<ALLEGRO_FONT, function<void(ALLEGRO_FONT*)>>;
	using tData = typename ExMin2Dfdtd::tData;

	ExMinTE2DRenderer(tEMFieldPtr __field_proc) : Bitmap2DRenderer(), field_proc{__field_proc},
			Nx(field_proc->getNx()), Ny(field_proc->getNy()){};

private:
	void attach(){
		font = tFont(al_create_builtin_font(),[=](ALLEGRO_FONT* ptr){al_destroy_font(ptr);});
		tBase::attach();
	};
	void detach(){
		tBase::detach();
		font = nullptr;
	};
	void draw_memory(){
		data = field_proc->copy_data();
		for(int i = 0; i < Nx; i++)
			for(int j = 0; j < Ny; j++){
				unsigned char c = 255*data[i][0].c.Ex/(double)Nx;
				al_put_pixel(i,j,al_map_rgb(c, c, c));
			}
		al_draw_line(0,0,getNx()-1,getNy()-1,al_map_rgb(255,0,0),0);
	};
	void draw_video(){al_draw_text(font.get(),al_map_rgb(0,255,0),5,10,0,"direct drawn text");};
	size_t& getNx(){return Nx;};
	size_t& getNy(){return Ny;};

private:
	tEMFieldPtr field_proc;
	tFont font;
	tData data;

	size_t Nx;
	size_t Ny;
};

int main(int argc, char** argv) {

	shared_ptr<ExMin2Dfdtd> field(new ExMin2Dfdtd(ExMin2Dfdtd::tInit{300,300}));
	Window w(unique_ptr<ExMinTE2DRenderer>(new ExMinTE2DRenderer(field)));
	w.create_window(300,300);

	while(1){
		if(!w.is_running()) break;
		field->evaluate();
	}

	std::cout << "exit" << std::endl;

	return 0;
}
