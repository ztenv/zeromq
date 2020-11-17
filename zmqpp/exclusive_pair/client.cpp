/**
 * @file client.cpp
 * @brief zmq_pair demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-17
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

DEFINE_string(end_point,"tcp://127.0.0.1:12345","the end_point connected by client");
DEFINE_int32(io_thread_count,2,"the io thread count of the zeromq context");
DEFINE_int32(send_interval,1000,"the interval of sending message to client");

bool send_message(zmqpp::socket &socket);
bool handle_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket pair(context,zmqpp::socket_type::pair);
    pair.connect(FLAGS_end_point);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(send_message,std::ref(pair)));
    looper.add(pair,std::bind(handle_message,std::ref(pair)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool send_message(zmqpp::socket &socket)
{
    zmqpp::message msg;
    std::string tp=common::format_time();
    msg.add(tp);

    auto res=socket.send(msg);
    if(res)
    {
        LOG_INFO("send:["<<tp<<"]");
    }

    return res;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;

    auto res=socket.receive(msg);
    if(res)
    {
        LOG_INFO("recv:["<<msg.get(0)<<"]");
    }

    return res;
}


