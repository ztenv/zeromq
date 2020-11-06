/**
 * @file server.cpp
 * @brief ZMQ_SERVER demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-03
 */

#include <iostream>

#include <gflags/gflags.h>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>

#include <zmqpp/socket.hpp>
#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>

#include "../include/common.h"

DEFINE_string(bind_endpoint,"tcp://*:12345","the endpoint that the server to bind");
DEFINE_int32(io_thread_count,1,"the io thread number of zeromq context");


bool handle_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    LOG_INFO("server is initializing...");

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket server_socket(context,zmqpp::socket_type::server);
    server_socket.bind(FLAGS_bind_endpoint);

    zmqpp::loop looper;
    looper.add(server_socket,std::bind(handle_message,std::ref(server_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();

    return 0;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;
    auto res=socket.receive(msg);

    if(res)
    {
        LOG_INFO("recv "<<" message:"<<msg.get(0));
        res=socket.send(msg);//reply msg to client
    }

    return res;
}


