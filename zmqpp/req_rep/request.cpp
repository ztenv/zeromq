#include <chrono>
#include <iostream>
#include <functional>

#include <gflags/gflags.h>
#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "common.h"

using namespace std;

DEFINE_string(server_endpoint,"tcp://127.0.0.1:55555","the server endpoint that connected by the client");
DEFINE_string(id,"req_socket","the socket id");

DEFINE_int32(req_interval,1000,"send request interval ms");
DEFINE_int32(io_thread_count,1,"the io thread count of zmq io_context");


bool send_msg(zmqpp::socket &socket);
bool receive_msg(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    //build zmq context
    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    //build zmq socket
    zmqpp::socket req_socket(context,zmqpp::socket_type::req);
    req_socket.set(zmqpp::socket_option::identity,FLAGS_id);
    req_socket.connect(FLAGS_server_endpoint);
    cout<<"connected to "<<FLAGS_server_endpoint<<endl;

    //build the loop event
    zmqpp::loop looper;
    //register periodical callback
    looper.add(std::chrono::milliseconds(FLAGS_req_interval),0,std::bind(send_msg,std::ref(req_socket)));
    //register io events:poll_in and poll_error
    looper.add(req_socket,std::bind(receive_msg,std::ref(req_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();

    return 0;
}

bool send_msg(zmqpp::socket &socket)
{
    bool res=true;

    auto time_str=common::format_time();
    zmqpp::message msg;
    msg.add(FLAGS_id); //the first frame
    msg.add(time_str); //the second frame

    res=socket.send(msg);
    if(res)
    {
        LOG_INFO("send ["<<time_str<<"]to["<<FLAGS_server_endpoint<<"]");
    }else{
        LOG_ERROR("sned ["<<time_str<<"]to["<<FLAGS_server_endpoint<<"] error");
    }

    return res;
}

bool receive_msg(zmqpp::socket &socket)
{
    bool res=true;
    zmqpp::message msg;

    res=socket.receive(msg);
    if(res)
    {
        LOG_INFO("recv ["<<msg.get(1)<<"]from["<<msg.get(0)<<"]");
    }else{
        LOG_ERROR("recv msg error,id="<<FLAGS_id);
    }

    return res;
}

