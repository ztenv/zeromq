#include <ctime>

#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

using namespace std;

DEFINE_string(id,"client","app id");
DEFINE_string(endpoint,"tcp://127.0.0.1:11111","endpoint to bind,endpoints must be splited by ,");
DEFINE_string(user_name,"","plain user name");
DEFINE_string(password,"","plain password");

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

bool send_msg(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    zmqpp::socket dealer(context,zmqpp::socket_type::dealer);
    if((FLAGS_user_name.length()>0)&&(FLAGS_password.length()>0))
    {
        dealer.set(zmqpp::socket_option::plain_username,FLAGS_user_name);
        dealer.set(zmqpp::socket_option::plain_password,FLAGS_password);
    }

    //dealer.set(zmqpp::socket_option::stream_notify,1);
    if (FLAGS_endpoint.find(",")!=std::string::npos)
    {
        auto endpoints=split(FLAGS_endpoint,",");
        for(auto &ep:endpoints)
        {
            cout<<"connecting..."<<ep<<endl;
            dealer.connect(ep);
        }
    }else{
        dealer.connect(FLAGS_endpoint);
    }

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(1000),0,std::bind(send_msg,std::ref(dealer)));
    looper.start();
    
    return 0;
}


bool send_msg(zmqpp::socket &socket)
{
    auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&t),"%Y-%m-%d %H:%M:%S");
    zmqpp::message msg;
    //msg.add("server1");
    msg.add(ss.str());
    bool res=socket.send(msg);

    cout<<FLAGS_id<<" send:"<<ss.str()<<",res="<<res<<endl;;
    return res;
}
