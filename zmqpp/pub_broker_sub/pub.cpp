/**
 * @file pub.cpp
 * @brief pub demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-06
 */

#include <chrono>
#include <iostream>
#include <string>

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(broker_endpoint,"tcp://127.0.0.1:15556","the broker backend endpoint that connected by pub server");
DEFINE_int32(pub_interval,1000,"publish interval,ms");
DEFINE_int32(io_thread_count,1,"the io_thread count of the zeromq context");
DEFINE_string(id,"pub","the server id");

bool pub_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket pub_socket(context,zmqpp::socket_type::pub);
    pub_socket.connect(FLAGS_broker_endpoint);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_pub_interval),0,std::bind(pub_message,std::ref(pub_socket)));
    looper.start();

    return 0;
}

bool pub_message(zmqpp::socket &socket)
{
    static unsigned long long index=0;

    zmqpp::message msg;
    if(index++%2==0)
    {
        msg.add("even");
    }else{
        msg.add("odd");
    }

    msg.add(FLAGS_id+":"+common::format_time());

    LOG_INFO(FLAGS_id<<" pub["<<msg.get(0)<<"],["<<msg.get(1)<<"]");

    auto res=socket.send(msg);
    return res;
}

