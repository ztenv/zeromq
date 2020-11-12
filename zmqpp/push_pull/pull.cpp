/**
 * @file pull.cpp
 * @brief pull demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-12
 */

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(connect_endpoint,"tcp://127.0.0.1:12345","the endpoint that connected by pull socket");
DEFINE_int32(io_thread_count,2,"the io thread count of the zeromq context");

bool handle_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket pull_socket(context,zmqpp::socket_type::pull);
    pull_socket.connect(FLAGS_connect_endpoint);

    zmqpp::loop looper;
    looper.add(pull_socket,std::bind(handle_message,std::ref(pull_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;
    bool res=socket.receive(msg);
    if(res)
    {
        LOG_INFO("recv["<< msg.get(0)<<"]");
    }

    return res;
}

