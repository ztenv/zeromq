#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>

#include <zmq.h>
#include <zmqpp/context.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include <gflags/gflags.h>

using namespace std;

DEFINE_string(id,"broker","app id");
DEFINE_string(sub_endpoint,"tcp://*:5555","xsub bind address");
DEFINE_string(pub_endpoint,"tcp://*:6666","xpub bind address");

void proxy();

atomic<bool> flag(true);
int main (int argc,char *argv[])
{
    google::SetUsageMessage("usage:");
    google::ParseCommandLineFlags(&argc,&argv,true);


    thread t=thread(std::bind(proxy));

    t.join();

    return 0;
}

void proxy()
{
    zmqpp::context context;
    zmqpp::socket xsub_socket(context,zmqpp::socket_type::xsub);
    xsub_socket.bind(FLAGS_sub_endpoint);

    string msg(1,0x1);
    xsub_socket.send(msg);

    zmqpp::socket xpub_socket(context,zmqpp::socket_type::xpub);
    xpub_socket.bind(FLAGS_pub_endpoint);
    xpub_socket.set(zmqpp::socket_option::xpub_verbose,1);//if flag is 1,xpub will send all subscribe msgs to upstream else xpub will send
    //new subscribe msgs to upstream
    
    zmqpp::poller poller;
    poller.add(xsub_socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    poller.add(xpub_socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);

    //cout<<poller.has(xsub_socket)<<endl;
    //cout<<poller.has(xpub_socket)<<endl;

    unsigned long long back_to_front=0;
    unsigned long long front_to_back=0;
    while(flag)
    {
        if(poller.poll())
        {
            auto e=poller.events(xsub_socket);
            if(e==zmqpp::poller::poll_in)
            {
                string msg;
                auto res=xsub_socket.receive(msg);
                //cout<<"backend recv:["<<msg<<"],res="<<res<<endl;
                res=xpub_socket.send(msg);
                //cout<<"front send:["<<msg<<"],res="<<res<<endl<<endl;

                if((++back_to_front%1000)==0)
                {
                    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    auto ss=std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                    cout<<"["<<ss<<"]back to front:"<<back_to_front<<" messages"<<endl;
                }
            } 

            e=poller.events(xpub_socket);
            if(e==zmqpp::poller::poll_in)
            {
                string msg;
                auto res=xpub_socket.receive(msg);
                //cout<<"front recv:["<<msg<<"],res="<<res<<endl;
                res=xsub_socket.send(msg);
                //cout<<"backend send:["<<msg<<"],res="<<res<<endl<<endl;
                if((++front_to_back%1000)==0)
                {
                    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    auto ss=std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                    cout<<"["<<ss<<"]front to back:"<<front_to_back<<" messages"<<endl;
                }
            }
        }
    }
    xsub_socket.close();
    xpub_socket.close();
}
