//
// Created by morrigan on 10/02/17.
//
#include <iostream>
#include <vector>
#include <algorithm>

#include <boost/circular_buffer.hpp>

#include "../../lib_dist/swp_fifo.hpp"

using namespace std;
using namespace boost;

const int lp = 2;
const int lq = 1;

list<vector<char>> in_p;
list<vector<char>> in_q;

void write_p(const vector<char>& data){
    cout << string(data.begin(),data.end()) << endl;
}

void write_q(const vector<char>& data){
    return;
}

vector<char> read_p(){
    vector<char> data;
    return data;
}

vector<char> read_q(){
    vector<char> data(100);
    for(int i = 0; i < 100; i++){
        char ch = (char)(rand() % 255);
        if(isprint(ch)) data.push_back(ch);
    }
    return data;
}

void send_p(const vector<char>& pkt){
    in_q.push_back(pkt);
}

void send_q(const vector<char>& pkt){
    in_p.push_back(pkt);
}

vector<char> receive_p(){
    vector<char> out(std::move(in_p.front()));
    in_p.pop_front();
    return out;
}

vector<char> receive_q(){
    vector<char> out(std::move(in_q.front()));
    in_q.pop_front();
    return out;
}

bool is_empty_p(){
    return in_p.empty();
}

bool is_empty_q(){
    return in_q.empty();
}

int main (){
    typedef SWP_FIFO<lp,lq> t_SWPp;
    typedef SWP_FIFO<lq,lp> t_SWPq;

    t_SWPp::IOFuncs Fp;
    Fp.read = &read_p;
    Fp.write = &write_p;
    Fp.send = &send_p;
    Fp.receive = &receive_p;
    Fp.is_empty = &is_empty_p;
    t_SWPp p(Fp);

    t_SWPq::IOFuncs Fq;
    Fq.read = &read_q;
    Fq.write = &write_q;
    Fq.send = &send_q;
    Fq.receive = &receive_q;
    Fq.is_empty = &is_empty_q;
    t_SWPq q(Fq);

    return 0;
}