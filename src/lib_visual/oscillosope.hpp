//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_OSCILLOSOPE_HPP
#define COMPPHYSFWK_OSCILLOSOPE_HPP

#include <gsl/gsl_interp.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "renderer.hpp"


//generic data structure for oscilloscope frame signal
struct OSC_INTERP_FRAME{
    vector<double> data_y;
    vector<double> data_x;

    shared_ptr<gsl_interp> interp{nullptr};
    shared_ptr<gsl_interp_accel> acc{nullptr};
};

//abstract interface to be implemented by actual oscilloscope source
class osc_src_interface{
public:
    virtual const shared_ptr<OSC_INTERP_FRAME> get_frame() = 0;
};

class Oscilloscope : public Bitmap2DRenderer{
public:
    Oscilloscope(shared_ptr<osc_src_interface> src) : Bitmap2DRenderer(), Nx{375*3}, Ny{300*3}, src{src} {
        v.resize(Nx);
        for(int i = 0; i < Nx; i++){
            v[i].x = i;
            v[i].color = al_map_rgb(255,0,0);
        }

        for(int i = -12; i <= 0; i++){
            scale.push_back(1*pow(10,(float)i));
            scale.push_back(2*pow(10,(float)i));
            scale.push_back(5*pow(10,(float)i));
            if(i < 0) v_scale_idx += 3;
            if(i < -7) t_scale_idx +=3;
        }
    };

protected:
    void draw_memory(){

        //draw grid
        for(int i = 1; i <= 9; i++) al_draw_line(i*Nx/10,0,i*Nx/10,Ny,al_map_rgb(25,25,25),1);
        for(int i = 1; i <= 7; i++) al_draw_line(0,i*Ny/8,Nx,i*Ny/8,al_map_rgb(25,25,25),1);

        al_draw_line(0, ((double)Ny/2+cv),Nx,((double)Ny/2+cv),al_map_rgb(25,25,255),1);

        //get frame
        auto data = src->get_frame();
        if(!data || data->data_y.empty() || data->data_x.empty()) return;



        //continuous limits (for interpolation)
        const double x_min = data->data_x.front();
        const double x_max = data->data_x.back();
        //discrete limits (for al_draw_prim)
        int start = 0, end = Nx;

        float y;
        for(int i = 0; i < Nx; i++){

            //simple time rediscretization
            double x = ((double)i - ts)*scale[t_scale_idx]*10/(dt*Nx);
            y = -1;
            if(x < x_min){
                start++;
            }else if(x > Nx){
                end--;
            }else{
                y = gsl_interp_eval(data->interp.get(), data->data_x.data(), data->data_y.data(), x, data->acc.get());
                y = (((double)Ny/((double)8*scale[v_scale_idx]))*y+(double)Ny/2+cv);
            }

            //truncate signal that is out of screen range
            if(y > Ny){
                v[i].color = al_map_rgba(255,0,0,100);
                v[i].u = 127;
                v[i].v = 127;
                v[i].y = Ny+1;
            }else if(y < 0){
                v[i].color = al_map_rgba(255,0,0,50);
                v[i].y = 0;
            }else{
                v[i].y = y;
                v[i].color = al_map_rgb(255,0,0);
            }

        }

        if(start < end) al_draw_prim(v.data(), nullptr, nullptr, start, end, ALLEGRO_PRIM_LINE_STRIP);

    }

    virtual void msg_proc(const ALLEGRO_EVENT& ev){
        if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
            mouse_down = true;
        }else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
            mouse_down = false;
        }else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
            if(mouse_down) {
                cv += ev.mouse.dy;
                ts += ev.mouse.dx;
            }
        }else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
            if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN){
                if(v_scale_idx > 0)v_scale_idx--;
            }else if(ev.keyboard.keycode == ALLEGRO_KEY_UP){
                if(v_scale_idx < scale.size()-1)v_scale_idx++;
            }else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT){
                if(t_scale_idx > 0)t_scale_idx--;
            }else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                if(t_scale_idx < scale.size()-1)t_scale_idx++;
            }

        }
    }

    size_t& getNx(){return Nx;};
    size_t& getNy(){return Ny;};

protected:
    size_t Nx; //screen width (10)
    size_t Ny; //screen height (8)
    double dt = 0.008e-6; //125Mhz
//    t_plain_data_ptr data = nullptr;
    vector<ALLEGRO_VERTEX> v;
    vector<double> scale; //scale factors
    shared_ptr<osc_src_interface> src;


    bool mouse_down = false;
    double cv = 0, ts = 0; //voltage center [volts], time start [sec]
    char v_scale_idx; //voltage scale [volts]
    char t_scale_idx; //time scale [sec]
};

#endif //COMPPHYSFWK_OSCILLOSOPE_HPP
