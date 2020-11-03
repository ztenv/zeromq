#include <ctime>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>

#include <zmqpp/auth.hpp>
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
DEFINE_string(front_endpoint,"tcp://*:1111","broker front endpoint");
DEFINE_string(server_endpoint,"tcp://127.0.0.1:22222","server's endpoint,multiple endpoints must be splited by ,");
DEFINE_string(username,"","plain user name");
DEFINE_string(password,"","plain password");

using namespace std;

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
    front_socket.set(zmqpp::socket_option::identity,FLAGS_front_endpoint);

    zmqpp::auth author(context);
    if((FLAGS_username.length()>0)&&(FLAGS_password.length()>0))
    {
        front_socket.set(zmqpp::socket_option::plain_server,1);
        author.configure_plain(FLAGS_username,FLAGS_password);
    }

    front_socket.bind(FLAGS_front_endpoint);

    zmqpp::socket back_socket(context,zmqpp::socket_type::router);
    if (FLAGS_server_endpoint.find(",")!=std::string::npos)
    {
        auto endpoints=split(FLAGS_server_endpoint,",");
        for(auto &ep:endpoints)
        {
            cout<<"connecting..."<<ep<<endl;
            back_socket.connect(ep);
        }
    }else{
        back_socket.connect(FLAGS_server_endpoint);
    }
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
                cout<<"front recv:["<<data<<"]from["<<msg.get(0)<<"],res="<<res<<endl;

                zmqpp::message smsg;
                smsg.add(msg.get(1));//server id
                smsg.add(msg.get(0));//client id
                smsg.add(msg.get(2));//message data
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
                cout<<"back recv:["<<data<<"],from["<<msg.get(0)<<"]res="<<res<<endl;

                zmqpp::message smsg;//The first frame is the trading agent router id;the second frame is the client router id;the third frame is the reply data.
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
