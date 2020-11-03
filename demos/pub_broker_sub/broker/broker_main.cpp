#include <ctime>

#include <iostream>
#include <chrono>
#include <sstream>
#include <thread>
#include <iomanip>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>
#include <zmqpp/proxy.hpp>

using namespace std;

DEFINE_string(xsub_endpoint,"tcp://*:5555","xsub endpoint");
DEFINE_string(xpub_endpoint,"tcp://*:6666","xpub endpoint");

DEFINE_string(route_endpoint,"tcp://*:7777","xpub endpoint");

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


void proxy_run(zmqpp::context &context);

//void router_proxy(zmqpp::context &context);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);
    zmqpp::context context;

    std::thread t=std::thread(std::bind(proxy_run,std::ref(context)));
    //std::thread t2=std::thread(std::bind(router_proxy,std::ref(context)));
    //t2.join();
    t.join();
    
    return 0;
}

void proxy_run(zmqpp::context &context)
{

    zmqpp::socket backend(context,zmqpp::socket_type::xsub);
    backend.set(zmqpp::socket_option::linger,1);
    backend.bind(FLAGS_xsub_endpoint);
    backend.send(string(1,0x1));

    zmqpp::socket frontend(context,zmqpp::socket_type::xpub);
    frontend.bind(FLAGS_xpub_endpoint);

    zmqpp::poller poller;
    poller.add(backend,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    poller.add(frontend,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    //cout<<poller.has(backend)<<":"<<poller.has(frontend)<<endl;

    unsigned long long pub_msg_count=0;

    while(true)
    {
        if(poller.poll())
        {
            auto e=poller.events(backend);
            if(e==zmqpp::poller::poll_in)
            {
                zmqpp::message msg;
                backend.receive(msg);
                //std::string data;
                //msg.get(data,0);
                bool res=frontend.send(msg);
                pub_msg_count+=1;
                //cout<<"recv msg from backend["<<data<<"],send to frontend res="<<res<<endl;

            }else if(e==zmqpp::poller::poll_error)
            {
                cout<<"poll backend error"<<endl;
            }

            if(pub_msg_count%1000==0)
            {
                auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::stringstream ss;
                ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                cout<<"["<<ss.str()<<"]forward "<<pub_msg_count<<" messages from backend to frontend"<<endl;
            }

            e=poller.events(frontend);
            if(e==zmqpp::poller::poll_in)
            {
                zmqpp::message msg;
                frontend.receive(msg);
                //cout<<"recv msg from frontend"<<endl;
                backend.send(msg);

            }else if(e==zmqpp::poller::poll_error)
            {
                cout<<"poll frontend error"<<endl;
            }
        }
    }
    backend.close();
    frontend.close();
}

//void router_proxy(zmqpp::context &context)
//{
//    zmqpp::socket router(context,zmqpp::socket_type::router);
//    router.set(zmqpp::socket_option::identity,"router");
//    router.bind(FLAGS_route_endpoint);
//
//    zmqpp::socket dealer(context,zmqpp::socket_type::dealer);
//    dealer.set(zmqpp::socket_option::identity,"broker");
//
//    //dealer will connect all online service's endpoint,this dependencies online notifycation
//
//    zmqpp::poller poller;
//    poller.add(router,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
//    poller.add(dealer,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
//
//    while(true)
//    {
//        if(poller.poll())
//        {
//            auto e=poller.events(dealer);
//            if(e==zmqpp::poller::poll_in)
//            {
//                zmqpp::message msg;
//                dealer.receive(msg);
//                cout<<"dealer recv:"<<msg.get(0)<<":"<<msg.get(1)<<":"<<endl;
//
//            }else if (e==zmqpp::poller::poll_error)
//            {
//                cout<<"poll dealer error"<<endl;
//            }
//
//            e=poller.events(router);
//            if(e==zmqpp::poller::poll_in)
//            {
//                zmqpp::message msg;
//                router.receive(msg);
//
//                //cout<<"router recv src:"<<msg.get(0)<<endl;
//                //cout<<"router recv dst::"<<msg.get(1)<<endl;
//                //cout<<"router recv content::"<<msg.get(2)<<endl;
//
//                zmqpp::message mmsg;
//                mmsg.add(msg.get(1));
//                mmsg.add(msg.get(2));
//                int res=dealer.send(mmsg,true);
//                cout<<"broker dealer send,res="<<res<<endl;
//            }
//        }
//    }
//
//    router.close();
//    dealer.close();
//}
