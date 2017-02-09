//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_SCPI_CLI_HPP
#define COMPPHYSFWK_SCPI_CLI_HPP

#include <iostream>
#include <vector>
#include <chrono>

#include <boost/asio.hpp>

#include "../../lib_dsp/pipeline.hpp"
#include "../../utils/performance.hpp"

using namespace std;
using namespace boost::asio::ip;

inline void sleep_ms(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
};

struct BIN_PKT : public MESSAGE{
    vector<char> bin_frame; //data
    string dt; //measured acquisition time
};

class scpi_client : public source<BIN_PKT>{
public:
    using tBase = source<BIN_PKT>;
    scpi_client(string ip_str, string port) : ip_str{ip_str}, port{port},
                                              tBase(nullptr/*main is overridden, so no matter*/,
                                                    QUEUE_POLICY::drop/*it is important to keep request queue on server,
                                              *so better drop binary data in the case when next filter is slower than
                                              *acquisition speed*/,
                                                    "scpi_client"){}
protected:
    virtual void main(){

        tcp::iostream s_tcp; //tcp channel
        string line; //string line for ASCII I/O
        ns_timer<size_t> timer; timer.res = 0; //measure transfer rate
        char c; //byte to read from input stream
        vector<char> ch_n_pts;//number of binary points to read

        //TODO: Find IP by known MAC
        //TODO: Set connection timeot
        //Connect to server and configure data acquisition
        s_tcp.connect(ip_str,port);

        cout << "connected to " << ip_str << endl;

        //configure acquisition mode

        s_tcp << "ACQ:RST;" << "\r\n";
        s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";
        //this is always ON by default
        s_tcp << "ACQ:AVG ON" << "\r\n";
        //can't understand how DEC is working,  this is not as simple as it is written in documentation
        //1024 has buffer length ~8.5ms
        s_tcp << "ACQ:DEC 1024" << "\r\n";/* 1(131mks)  8(1ms)  64(8.4ms)  1024(134ms)  8192(1s)  65536 */

        s_tcp << "ACQ:AVG?" << "\r\n";
        s_tcp >> line;
        cout << "Is acquire averaged? " << line << endl;

        //after trigger we have to wait until this number of points to be captured
        //so far we would know that the buffer was completely renewed and no delay after
        //ACQ:START is needed
        s_tcp << "ACQ:TRIG:DLY 16384"  << "\r\n";
        s_tcp << "ACQ:TRIG:DLY?"  << "\r\n";
        s_tcp >> line;
        cout << "Trigger delay (samples) " << line << endl;

//        s_tcp << "ACQ:TRIG:LEV 300" << "\r\n";


        int requests_pending = 0;
        bool stop = false;
        while(1){

            //check for terminate condition (fast, unblocking pull)
            t_IN_PTR curr_in = pull_msg(false);
            if(curr_in && curr_in->cmd == CMD_MSG::stop) stop = true;

            //start measuring data transfer rate
            timer.start();

            //request for single data block
            if(!stop){

                s_tcp << "ACQ:START" << "\r\n";
                s_tcp << ":ACQ:TRIG CH1_PE;" << "\r\n";

                while(1){
                    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
                    s_tcp >> line;
                    cout << line << "; ";
                    if (line.find("TD") != string::npos){ cout << endl; break; }
                }

                s_tcp << "ACQ:SOUR1:DATA?" << "\r\n";

                requests_pending++;
                //DO NOT keep small request queue on the server (speed boost x2) - old code
                if(requests_pending < 1) continue;
            }else if(requests_pending == 0){
                //get out from acquisition loop only after input queue
                //was read completely
                break;
            }

            //read byte by byte until data start delimiter
            do{ s_tcp.read(&c,1); } while(c != '#');

            //read number of chars describing data length
            s_tcp.read(&c,1);
            auto l = stoi(string(1,c));

            //read data length and convert it to numeric type
            ch_n_pts.clear();
            ch_n_pts.resize(l+1);
            s_tcp.read(ch_n_pts.data(),l);
            ch_n_pts.at(l) = 0;
            //TODO: check size is correct (multiple of 4, less than buffer), apply asserts
            auto dl = stoi(string(ch_n_pts.data()));


            //read points
            shared_ptr<BIN_PKT> pkt(new BIN_PKT);
            pkt->bin_frame.resize(dl);
            s_tcp.read(static_cast<char*>((void*)pkt->bin_frame.data()),dl);
            requests_pending--;

            timer.stop();
            pkt->dt = timer.dt();
            //send data to the next processing node
            if(next){
                next->put(move(pkt),pol);
            }else{
                pkt.reset();
                cerr << name << " no destination, data lost" << endl;
            }
        }  // end_while

        s_tcp.flush();
        cout << "Reset server, close connection" << endl;
        s_tcp << "ACQ:RST" << "\r\n";
        s_tcp.flush();

        s_tcp.close();
    }
private:
    string ip_str;
    string port;
};

#endif //COMPPHYSFWK_SCPI_CLI_HPP
