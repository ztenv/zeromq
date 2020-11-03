#include <ctime>

#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>

#include <gflags/gflags.h>

#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

using namespace std;

DEFINE_string(endpoint,"tcp://127.0.0.1:6666","connect endpoint");
//DEFINE_string(router_endpoint,"tcp://127.0.0.1:7777","connect endpoint");
//DEFINE_string(dealer_id,"client","dealer socket's identify name");
DEFINE_string(server_id,"server","server socket's identify name");
DEFINE_string(topics,"","subscribes topics,splited by ,,such as :pub1,pub2,pub3");
DEFINE_int32(interval,1000,"request interval,ms");

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

unsigned long long recv_msg_count=0;
bool recv_msg(zmqpp::socket &socket);
//bool request(zmqpp::socket &socket);
//bool recv_reply(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);


    zmqpp::context context;
    zmqpp::socket sub_socket(context,zmqpp::socket_type::sub);
    sub_socket.connect(FLAGS_endpoint);

    cout<<"connect to "<<FLAGS_endpoint<<endl;
    cout<<"sub topics:"<<FLAGS_topics<<endl;
    if(FLAGS_topics.find(",")!=std::string::npos)
    {
        auto topics=split(FLAGS_topics,",");
        for(auto & topic:topics)
        {
            sub_socket.subscribe(topic);
        }
    }else{
        sub_socket.subscribe(FLAGS_topics);
    }


    //zmqpp::socket dealer_socket(context,zmqpp::socket_type::dealer);
    //dealer_socket.set(zmqpp::socket_option::identity,FLAGS_dealer_id);
    //dealer_socket.connect(FLAGS_router_endpoint);


    zmqpp::loop looper;
    //looper.add(std::chrono::milliseconds(FLAGS_interval),0,std::bind(request,std::ref(dealer_socket)));
    //looper.add(dealer_socket,std::bind(recv_reply,std::ref(dealer_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.add(sub_socket,std::bind(recv_msg,std::ref(sub_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool recv_msg(zmqpp::socket &socket)
{
    zmqpp::message msg;
    bool res=socket.receive(msg);
    if(++recv_msg_count%1000==0)
    {
        std::stringstream ss;
        auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");

        cout<<"recv "<<recv_msg_count<<" msgs"<<endl;
        cout<<"["<<ss.str()<<"]recv topic["<<msg.get(0)<<"],["<<msg.get(1)<<"]"<<endl;
    }
    return res;
}

//bool request(zmqpp::socket &socket)
//{
//
//    std::stringstream ss;
//    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//    ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
//
//    zmqpp::message msg;
//    msg.add("server");
//    msg.add(ss.str());
//    cout<<msg.get(0)<<":"<<msg.get(1)<<endl;
//    bool res=socket.send(msg);
//
//    cout<<"dealer send:"<<ss.str()<<endl;
//    return res;
//}
//
//bool recv_reply(zmqpp::socket &socket)
//{
//    zmqpp::message msg;
//    bool res=socket.receive(msg);
//    cout<<"dealer recv:"<<msg.get(0)<<endl;
//
//    return res;
//}
