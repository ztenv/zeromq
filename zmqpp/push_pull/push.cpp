/**
 * @file push.cpp
 * @brief push demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-12
 */

#include <chrono>

#include <gflags/gflags.h>
#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(bind_endpoint,"tcp://*:12345","the endpoint binded by push socket");
DEFINE_int32(io_thread_count,1,"the io thread count of the zeromq context");
DEFINE_int32(send_interval,1000,"the sending interval ,ms");

bool send_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket push_socket(context,zmqpp::socket_type::push);
    push_socket.bind(FLAGS_bind_endpoint);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(send_message,std::ref(push_socket)));
    looper.start();
    
    return 0;
}

bool send_message(zmqpp::socket &socket)
{
    auto tm=common::format_time();
    zmqpp::message msg;
    msg.add(tm);

    bool res=socket.send(msg);
    if(res)
    {
        LOG_INFO("send["<<tm<<"]");
    }else{
        LOG_ERROR("send error");
    }

    return res;
}

