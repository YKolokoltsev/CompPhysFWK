//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_SCPI_CLI_HPP
#define COMPPHYSFWK_SCPI_CLI_HPP

#include <iostream>
#include <vector>

#include <boost/asio.hpp>

#include "../../lib_dsp/pipeline.hpp"
#include "../../utils/performance.hpp"

using namespace std;
using namespace boost::asio::ip;

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

        //TODO: Detect IP by known MAC
        //Connect to server and configure data acquisition
        s_tcp.connect(ip_str,port);

        //configure acquisition mode
        s_tcp << "ACQ:DEC 1;" << "\r\n";
        s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";

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

                s_tcp << "ACQ:START;" << "\r\n";
               // s_tcp << ":ACQ:TRIG CH1_PE;" << "\r\n";

                s_tcp << "ACQ:SOUR1:DATA?" << "\r\n";
                requests_pending++;
                //keep small request queue on the server (speed boost x2)
                if(requests_pending < 2) continue;
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
        }

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
