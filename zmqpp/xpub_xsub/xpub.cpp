/**
 * @file xpub.cpp
 * @brief xpub demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-05
 */

#include <chrono>
#include <iostream>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>


#include "../include/common.h"

using namespace std;

DEFINE_string(pub_endpoint,"tcp://*:12345","the endpoint binded by xpublish socket");
DEFINE_int32(pub_interval,1000,"the interval for publishing message");
DEFINE_int32(io_thread_count,1,"the io thread number of zeromq context");

bool handle_subscribe(zmqpp::socket &socket);
bool publish_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket xpub_socket=zmqpp::socket(context,zmqpp::socket_type::xpub);
    xpub_socket.bind(FLAGS_pub_endpoint);
    //pub_socket.connect("tcp://127.0.0.1:12346");

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_pub_interval),0,std::bind(publish_message,std::ref(xpub_socket)));
    looper.add(xpub_socket,std::bind(handle_subscribe,std::ref(xpub_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();

    return 0;
}

bool handle_subscribe(zmqpp::socket &socket)
{
    zmqpp::message msg;
    auto res=socket.receive(msg);
    LOG_INFO("handle subscribe msg:"<<msg.get(0));
    return res;
}

unsigned long long publish_count=0;

bool publish_message(zmqpp::socket &socket)
{
    zmqpp::message msg;

    if(publish_count++%2==0)
    {
        msg.add("even");//publish topic
    }else{
        msg.add("odd");//publish topic
    }
    msg.add(common::format_time());//publish data

    LOG_INFO("publish:["<<msg.get(0)<<"],["<<msg.get(1)<<"]");

    auto res=socket.send(msg);

    return res;
}

