#include <ctime>
#include <cstring>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>
#include <random>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include <common.h>

using namespace std;

DEFINE_int32(interval,1000,"pub message interval,it is ms");
DEFINE_string(endpoint,"tcp://*:5555","endpoint to bind");
DEFINE_string(instruments,"101010.cro","instruments to pub,splited by ,");

vector<string> split(const string &str,const string &pattern)
{
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }
    delete[] strc;
    return resultVec;
}

bool pub_msg(zmqpp::socket &socket);

std::default_random_engine e;
//std::uniform_real_distribution<double> d(1.0,100.0);
std::uniform_int_distribution<unsigned char> d(0,255);
unsigned long long send_msg_count=0;

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);


    zmqpp::context context;
    zmqpp::socket pub_socket(context,zmqpp::socket_type::pub);
    pub_socket.bind(FLAGS_endpoint);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_interval),0,std::bind(pub_msg,std::ref(pub_socket)));
    looper.start();
    pub_socket.close();
    
    return 0;
}

zmqpp::message build_md_msg(std::string instr)
{
    zmqpp::message msg;
    auto info=split(instr,".");
    std::stringstream ss;
    ss<<"md."<<info[1]<<"."<<info[0];
    
    //auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
    //string data(ss.str());
    msg.add(ss.str()); //add topic frame

    snapshot md_ss;
    strncpy(md_ss.instr,info[0].c_str(),sizeof(md_ss.instr)-1);
    strncpy(md_ss.exchange,info[1].c_str(),sizeof(md_ss.exchange)-1);
    md_ss.high=d(e);
    md_ss.open=d(e);
    md_ss.low=d(e);
    md_ss.close=d(e);

    string data;
    data.append((const char *)(&md_ss),sizeof(snapshot));
    msg.add(data); //add data frame

    return std::move(msg);
}

bool pub_msg(zmqpp::socket &socket)
{
    auto instrs=split(FLAGS_instruments,",");
    for(auto & item:instrs)
    {
        zmqpp::message msg=build_md_msg(item);
        string data;
        msg.get(data,0);
        bool res=socket.send(msg);
        ++send_msg_count;
        //if(++send_msg_count%1000==0)
        {
            std::stringstream ss;
            auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
            cout<<"pub "<<send_msg_count<<" msgs"<<endl;
            cout<<"["<<ss.str()<<"]pub->"<<data<<",res="<<res<<endl;
        }
    }
    return true;
}
