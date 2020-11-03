#include <cstring>
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

DEFINE_string(pub_endpoint,"tcp://127.0.0.1:5555","pub endpoints to connect,multiple endpoints must be splited by ,");

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
    if(FLAGS_pub_endpoint.find(",")!=std::string::npos)
    {
        auto endpoints=split(FLAGS_pub_endpoint,",");
        for(auto &ep:endpoints)
        {
            backend.connect(ep);
        }
    }else{
        backend.connect(FLAGS_pub_endpoint);
    }

    backend.send(string(1,0x1));//subscribe all topics

    zmqpp::poller poller;
    poller.add(backend,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    //poller.add(frontend,zmqpp::poller::poll_in|zmqpp::poller::poll_error);
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
                pub_msg_count+=1;
                //cout<<"recv msg from backend["<<data<<"],send to frontend res="<<res<<endl;
                //
                auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::stringstream ss;
                ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
                cout<<"["<<ss.str()<<"]recv["<<msg.get(0)<<":"<<msg.get(1)<<"]"<<pub_msg_count<<" messages from backend to frontend"<<endl;

            }else if(e==zmqpp::poller::poll_error)
            {
                cout<<"poll backend error"<<endl;
            }
        }
    }
    backend.close();
}
