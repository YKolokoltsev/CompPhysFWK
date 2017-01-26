//
// Created by morrigan on 1/21/17.
//

#include <iostream>
#include <memory>
#include <list>
#include <thread>

#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>

#include "../../utils/performance.hpp"
#include "../../lib_visual/renderer.hpp"
#include "../../lib_visual/window.hpp"

const size_t Npts = pow(2,14.0); //num of data points

using namespace std;
using namespace boost::asio::ip;

using t_data_ptr = shared_ptr<array<float,Npts>>;
list<t_data_ptr> adq_io;
bool stop = false;

/*
 * REDPITAYA data acquisition loop
 * */

void acquisition_proc(){
    tcp::iostream s_tcp; //tcp channel
    string line; //string line for ASCII I/O
    ns_timer<size_t> timer; timer.res = 0; //measure transfer rate
    char c; //byte to read from input stream
    boost::circular_buffer<char> cb(7); //data pack delimiter
    t_data_ptr data_ptr(new array<float,Npts>());//complete data line

    //Connect to server and configure data acquisition
    s_tcp.connect("192.168.100.4","5000");

    s_tcp << "ACQ:DEC 1;" << "\r\n";
    //s_tcp << "ACQ:TRIG:DLY 500;:ACQ:TRIG:LEV 30;" << "\r\n";
    s_tcp << "ACQ:START;" << "\r\n";
    //s_tcp << ":ACQ:TRIG CH2_PE;" << "\r\n";

    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
    s_tcp >> line;
    cout << line << endl;

    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
    s_tcp >> line;
    cout << line << endl;

    s_tcp << "ACQ:GET:DATA:UNITS RAW" << "\r\n";
    s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";

    //queue two data requests to the server, an answer onto the first one
    //will start the nonstop loop of subsequent requests sent during response transfer
    s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
    s_tcp << "ACQ:BUF:SIZE?" << "\r\n"; //request for delimiter line
    s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
    s_tcp << "ACQ:BUF:SIZE?" << "\r\n"; //...

    timer.start();
    bool local_stop = false;
    while(1){

        //read points
        s_tcp.read(static_cast<char*>((void*)data_ptr->data()),data_ptr->size()*4);

        //continue reading until the delimiter
        do{
            s_tcp.read(&c,1);
            cb.push_back(c);
        }while(!(cb[0] == '1' && cb[1] == '6' && cb[2] == '3' && cb[3] == '8' && cb[4] == '4' && cb[5] == '\r' && cb[6] == '\n'));

        timer.stop();
        adq_io.push_back(data_ptr); //(send data to main app) is it thread safe?

        if(local_stop) break;
        if(stop) {local_stop = true; continue;} //read data left in queue before break this loop

        data_ptr.reset(new array<float,Npts>());
        //cout << "BLOCK RECEIVED " << timer << endl;
        s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
        s_tcp << "ACQ:BUF:SIZE?" << "\r\n";
        timer.start();
    }

    cout << "Reset server, close connection" << endl;
    s_tcp << "ACQ:RST" << "\r\n";
    s_tcp.flush();

    s_tcp.close();
}

class Oscilloscope : public Bitmap2DRenderer{
public:
    Oscilloscope() : Bitmap2DRenderer(), Nx{375}, Ny{300} {
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

        //pop signal
        if(!adq_io.empty()) {
            data = adq_io.back();
            adq_io.clear();
        }

        if(!data) return;

        for(int i = 0; i < Nx; i++){
            int idx = (i - ts)*scale[t_scale_idx]*10/(dt*Nx);
            float y = 0;
            if(idx >= 0 && idx < Nx) y = (((double)Ny/((double)8*scale[v_scale_idx]))*data->data()[idx]+(double)Ny/2+cv);

            v[i].y = y>=0 && y < Ny ? y : 0;
            v[i].color = al_map_rgb(255,0,0); //why?
        }

        al_draw_prim(v.data(), nullptr, nullptr, 0, Nx, ALLEGRO_PRIM_LINE_STRIP);

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
    size_t Nx = 375; //screen width (10)
    size_t Ny = 300; //screen height (8)
    double dt = 0.008e-6; //125Mhz
    t_data_ptr data = nullptr;
    vector<ALLEGRO_VERTEX> v;
    vector<double> scale; //scale factors


    bool mouse_down = false;
    double cv = 0, ts = 0; //voltage center [volts], time start [sec]
    char v_scale_idx; //voltage scale [volts]
    char t_scale_idx; //time scale [sec]
};

int main(){

    //create scope window
    Window w(unique_ptr<Oscilloscope>(new Oscilloscope()));
    w.create_window(375,300);

    //after this line we get 16k points each 60ms
    thread adq(&acquisition_proc);

    while(1){
        if(!w.is_running()) break;
    }

    //stop gracefully acquisition thread on any symbol enter
    stop = true;
    adq.join();

    return 0;
}

