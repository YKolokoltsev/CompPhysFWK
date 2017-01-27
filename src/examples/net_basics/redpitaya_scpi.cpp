//
// Created by morrigan on 1/21/17.
//

#include <iostream>
#include <memory>
#include <list>
#include <thread>
#include <bitset>

#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include "../../utils/performance.hpp"
#include "../../lib_visual/renderer.hpp"
#include "../../lib_visual/window.hpp"

const size_t Npts = pow(2,14.0); //num of data points

using namespace std;
using namespace boost::asio::ip;

using t_plain_data = vector<char>;
using t_plain_data_ptr = shared_ptr<t_plain_data>;
list<t_plain_data_ptr> adq_io;
bool stop = false;

/*
 * REDPITAYA data acquisition loop
 * */

void acquisition_proc(){
    tcp::iostream s_tcp; //tcp channel
    string line; //string line for ASCII I/O
    ns_timer<size_t> timer; timer.res = 0; //measure transfer rate
    char c; //byte to read from input stream
    t_plain_data_ptr data_ptr(new t_plain_data());//complete data line
    vector<char> ch_n_pts;//number of binary points to read

    //Connect to server and configure data acquisition
    s_tcp.connect("192.168.100.4","5000");

    s_tcp << "ACQ:DEC 1;" << "\r\n";
    s_tcp << "ACQ:TRIG:DLY 500;:ACQ:TRIG:LEV 30;" << "\r\n";
    s_tcp << "ACQ:START;" << "\r\n";
    //s_tcp << ":ACQ:TRIG CH2_PE;" << "\r\n";

    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
    s_tcp >> line;
    cout << line << endl;

    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
    s_tcp >> line;
    cout << line << endl;

    //s_tcp << "ACQ:GET:DATA:UNITS RAW" << "\r\n";
    s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";

   // s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";

    bool local_stop = false;
    do{
        //start measuring data transfer rate
        timer.start();

        //request for single data block
        s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";

        //read until data start delimiter
        do{ s_tcp.read(&c,1); } while(c != '#');

        //read number of chars describing data length
        s_tcp.read(&c,1);
        auto l = stoi(string(1,c));

        //read and convert to numeric the data length
        ch_n_pts.clear();
        ch_n_pts.resize(l+1);
        s_tcp.read(ch_n_pts.data(),l);
        ch_n_pts.at(l) = 0;
        auto dl = stoi(string(ch_n_pts.data()));

        //read points, TODO: BUG on size of vector to read, shell be more generic
        data_ptr.reset(new t_plain_data());
        data_ptr->resize(dl);
        s_tcp.read(static_cast<char*>((void*)data_ptr->data()),dl);

        //(send data to main app) is it thread safe?
        adq_io.push_back(data_ptr);

        timer.stop();
        timer.res = dl;
        cout << "BLOCK RECEIVED "<< timer << endl;
    }while(!stop);

    s_tcp.flush();
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

        for(int i = 0; i < 128; i+=4)
        {

            float v;
            memcpy((void*)&v,(void*)&data->data()[i],2);

            cout << bitset<8>(data->data()[i]) << " "
                 << bitset<8>(data->data()[i+1]) << " "
                 << bitset<8>(data->data()[i+2]) << " "
                 << bitset<8>(data->data()[i+3]) << "    " << v;

            int i1 = data->data()[i+1]; i1 = i1 << 8;
            int i2 = data->data()[i+2];
            i1 = i1 + i2;
            cout << "   " << i1 << endl;
        }


        /*for(int i = 0; i < Nx; i++){
            int idx = (i - ts)*scale[t_scale_idx]*10/(dt*Nx);
            float y = 0;
            if(idx >= 0 && idx < Nx){
                //y = ntohl(data->data()[idx]);
                y = data->data()[idx];//*1e-6;

                //cout << idx << " " << bitset<32>(y) << endl;
                y = (((double)Ny/((double)8*scale[v_scale_idx]))*y+(double)Ny/2+cv);
            }

            v[i].y = y>=0 && y < Ny ? y : 0;
            v[i].color = al_map_rgb(255,0,0); //why?
        }

        al_draw_prim(v.data(), nullptr, nullptr, 0, Nx, ALLEGRO_PRIM_LINE_STRIP);*/

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
    t_plain_data_ptr data = nullptr;
    vector<ALLEGRO_VERTEX> v;
    vector<double> scale; //scale factors


    bool mouse_down = false;
    double cv = 0, ts = 0; //voltage center [volts], time start [sec]
    char v_scale_idx; //voltage scale [volts]
    char t_scale_idx; //time scale [sec]
};

using namespace boost::multiprecision;

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

