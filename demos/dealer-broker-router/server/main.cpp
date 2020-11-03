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


DEFINE_string(id,"server","app id");
DEFINE_string(endpoint,"tcp://*:11111","endpoint that server binds");
DEFINE_int32(send_interval,1000,"send request interval,ms");
DEFINE_string(username,"","plain user name");
DEFINE_string(password,"","plain password");

using namespace std;

bool recv_msg(zmqpp::socket *socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;

    zmqpp::socket router_socket(context,zmqpp::socket_type::router);
    router_socket.set(zmqpp::socket_option::identity,FLAGS_id);
    router_socket.bind(FLAGS_endpoint);

    zmqpp::loop looper;
    looper.add(router_socket,std::bind(recv_msg,&router_socket),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool recv_msg(zmqpp::socket *socket)
{
    zmqpp::message msg;
    bool res=socket->receive(msg);
    cout<<"recv["<<msg.get(2)<<"]from["<<msg.get(1)<<"],broker["<<msg.get(0)<<"],res="<<res<<endl;

    zmqpp::message smsg;
    smsg.add(msg.get(0));//broker router id
    smsg.add(msg.get(1));//client id
    smsg.add("reply from["+FLAGS_id+"]"+msg.get(2));//replay data
    socket->send(smsg);
    cout<<"reply["<<msg.get(1)<<"]to["<<msg.get(1)<<"],broker["<<msg.get(1)<<"],res="<<res<<endl;

    return true;
}
