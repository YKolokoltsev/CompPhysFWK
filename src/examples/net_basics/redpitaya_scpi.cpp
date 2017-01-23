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

using namespace std;
using namespace boost::asio::ip;

using t_data_ptr = shared_ptr<std::list<char>>;
list<t_data_ptr> adq_io;
bool stop = false;

/*
 * REDPITAYA data acquisition loop
 * */

void acquisition_proc(){
    tcp::iostream s_tcp; //tcp channel
    string line; //string line for ASCII I/O
    ns_timer<size_t> timer; //measure transfer rate
    char c; //byte to read from input stream
    boost::circular_buffer<char> cb(7); //data pack delimiter
    t_data_ptr data_ptr(new std::list<char>);//complete data line

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

    s_tcp << "ACQ:GET:DATA:UNITS RAW" << "\r\n";
    s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";

    //queue two data requests to the server, an answer onto the first one
    //will start the nonstop loop of subsequent requests sent during response transfer
    s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
    s_tcp << "ACQ:BUF:SIZE?" << "\r\n"; //request for delimiter line
    s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
    s_tcp << "ACQ:BUF:SIZE?" << "\r\n"; //...

    timer.start();
    while(1){
        s_tcp.read(&c,1);

        data_ptr->push_back(c);
        cb.push_back(c);
        //check delimiter presence (delimiter is just a buffer size response string that is always the same)
        if(cb[0] == '1' && cb[1] == '6' && cb[2] == '3' && cb[3] == '8' && cb[4] == '4' && cb[5] == '\r' && cb[6] == '\n'){
            timer.stop();
            data_ptr->erase(std::prev(data_ptr->end(),7),data_ptr->end()); //remove delimiter from data
            adq_io.push_back(move(data_ptr)); //(send data to main app) is it thread safe?
            if(stop) break;

            data_ptr.reset(new std::list<char>);
            cout << "BLOCK RECEIVED " << timer << endl;
            s_tcp << "ACQ:SOUR2:DATA?" << "\r\n";
            s_tcp << "ACQ:BUF:SIZE?" << "\r\n";
            timer.start();
        }
    }

    cout << "Reset server, close connection" << endl;
    s_tcp << "ACQ:RST" << "\r\n";
    s_tcp >> line; //clear buffer required, try to boost::asio::async_read_until (this line is buggy)

    s_tcp.close();
}

class Osciloscope : public Bitmap2DRenderer{
public:
    Osciloscope() : Bitmap2DRenderer(), Nx{300}, Ny{300} {};

protected:
    void draw_memory(){
        if(adq_io.empty()) return;
        auto data = *adq_io.front();
        adq_io.pop_front();

        cout << "Osc size:" << data.size() << endl;
    };

    size_t& getNx(){return Nx;};
    size_t& getNy(){return Ny;};

protected:
    size_t Nx;
    size_t Ny;
};

int main(){

    //create scope window
    Window w(unique_ptr<Osciloscope>(new Osciloscope()));
    w.create_window(300,300);

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

