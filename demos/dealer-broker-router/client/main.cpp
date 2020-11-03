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


DEFINE_string(id,"client","app id");
DEFINE_string(server_id,"server","which server to process the request msg");
DEFINE_string(endpoint,"tcp://127.0.0.1:1111","broker frontend endpoint");
DEFINE_int32(send_interval,1000,"send request interval,ms");
DEFINE_string(username,"","plain user name");
DEFINE_string(password,"","plain password");


using namespace std;

bool send_msg(zmqpp::socket *socket);
bool recv_msg(zmqpp::socket *socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;

    zmqpp::socket dealer_socket(context,zmqpp::socket_type::dealer);
    dealer_socket.set(zmqpp::socket_option::identity,FLAGS_id);
    if((FLAGS_username.length()>0)&&(FLAGS_password.length()>0))
    {
        dealer_socket.set(zmqpp::socket_option::plain_username,FLAGS_username);
        dealer_socket.set(zmqpp::socket_option::plain_password,FLAGS_password);
    }
    dealer_socket.connect(FLAGS_endpoint);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(send_msg,&dealer_socket));
    looper.add(dealer_socket,std::bind(recv_msg,&dealer_socket),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool send_msg(zmqpp::socket *socket)
{
    auto time=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&time),"%Y-%m-%d %H:%M:%S");
    string msg(ss.str());

    zmqpp::message mmsg;
    mmsg.add(FLAGS_server_id);
    mmsg.add(msg);
    bool res=socket->send(mmsg);
    cout<<"send:["<<msg<<"]-->["<<FLAGS_server_id<<"],res="<<res<<endl;

    return true;
}

bool recv_msg(zmqpp::socket *socket)
{
    string msg;
    bool res=socket->receive(msg);
    cout<<"recv:["<<msg<<"],res="<<res<<endl;
    return true;
}
