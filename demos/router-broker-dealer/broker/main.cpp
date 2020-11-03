#include <ctime>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>

#include <zmqpp/context.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include <gflags/gflags.h>


DEFINE_string(id,"broker","app id");
DEFINE_string(front_endpoint,"tcp://*:2222","broker front endpoint");
DEFINE_string(back_endpoint,"tcp://*:3333","broker back endpoint");
DEFINE_int32(send_interval,10000,"send request interval,ms");


using namespace std;

void proxy();

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    std::thread t=thread(std::bind(proxy));
    t.join();

    return 0;
}

void proxy()
{
    zmqpp::context context;
    zmqpp::socket front_socket(context,zmqpp::socket_type::router);
    front_socket.bind(FLAGS_front_endpoint);
    zmqpp::socket back_socket(context,zmqpp::socket_type::router);
    back_socket.bind(FLAGS_back_endpoint);

    zmqpp::poller poller;
    poller.add(front_socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    poller.add(back_socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);

    unsigned long long back_to_front=0;
    unsigned long long front_to_back=0;
    while(true)
    {
        if(poller.poll())
        {
            auto e=poller.events(front_socket);
            if(e==zmqpp::poller::poll_in)
            {
                zmqpp::message msg;
                string data;
                auto res=front_socket.receive(msg);
                msg.get(data,2);
                cout<<"front recv:["<<data<<"],res="<<res<<endl;

                zmqpp::message smsg;
                smsg.add(msg.get(1));
                smsg.add(msg.get(0));
                smsg.add(msg.get(2));
                res=back_socket.send(smsg);
                cout<<"back send to ["<<msg.get(1)<<"]:["<<data<<"],res="<<res<<endl<<endl;

                if((++back_to_front%1000)==0)
                {
                    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    auto ss=std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                    cout<<"["<<ss<<"]back to front:"<<back_to_front<<" messages"<<endl;
                }
            } 

            e=poller.events(back_socket);
            if(e==zmqpp::poller::poll_in)
            {
                zmqpp::message msg;
                string data;
                auto res=back_socket.receive(msg);
                msg.get(data,2);
                cout<<"back recv:["<<data<<"],res="<<res<<endl;

                zmqpp::message smsg;
                smsg.add(msg.get(1));
                smsg.add(msg.get(2));
                res=front_socket.send(smsg);
                cout<<"front send to ["<<msg.get(1)<<"]:["<<data<<"],res="<<res<<endl<<endl;
                if((++front_to_back%1000)==0)
                {
                    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    auto ss=std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                    cout<<"["<<ss<<"]front to back:"<<front_to_back<<" messages"<<endl;
                }
            }
        }
    }
    front_socket.close();
    back_socket.close();
}
