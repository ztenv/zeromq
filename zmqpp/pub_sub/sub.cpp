/**
 * @file sub.cpp
 * @brief sub demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-04
 */

#include <iostream>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(connect_endpoint,"tcp://127.0.0.1:12345","the endpoint binded by publish socket");
DEFINE_string(topic,"","the subscribe topic,there are 3 choices:odd even and empty topic is the default topic");
DEFINE_int32(io_thread_count,1,"the io thread number of zeromq context");

using namespace std;

bool handle_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket sub_socket(context,zmqpp::socket_type::sub);
    sub_socket.connect(FLAGS_connect_endpoint);
    //sub_socket.bind("tcp://*:12346");
    sub_socket.subscribe(FLAGS_topic);

    zmqpp::loop looper;
    looper.add(sub_socket,std::bind(handle_message,std::ref(sub_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;
    auto res=socket.receive(msg);

    LOG_INFO("recv topic:["<<msg.get(0)<<"],msg:["<<msg.get(1)<<"]from["<<FLAGS_connect_endpoint<<"]");
    return res;
}
