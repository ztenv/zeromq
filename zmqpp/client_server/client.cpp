/**
 * @file client.cpp
 * @brief ZMQ_CLIENT demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-03
 */

#include <chrono>
#include <iostream>

#include <gflags/gflags.h>

#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(connect_endpoint,"tcp://127.0.0.1:12345","the endpoint that the server to bind");
DEFINE_int32(io_thread_count,1,"the io thread number of zeromq context");
DEFINE_int32(send_interval,1000,"the send message interval,ms");

bool send_messae(zmqpp::socket &socket);
bool handle_message(zmqpp::socket &socket)

int main(int argc, char *argv[])
{
    LOG_INFO("server is initializing...");

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket client_socket(context,zmqpp::socket_type::client);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(send_messae,std::ref(client_socket)));
    looper.add(client_socket,std::bind(handle_message,std::ref(client_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();

    return 0;
}

bool send_messae(zmqpp::socket &socket)
{
    zmqpp::message msg;
    msg.add(common::format_time());
    auto res=socket.send(msg);
    LOG_INFO("send["<<msg.get(0)<<"]to["<<FLAGS_connect_endpoint<<"]");
    return res;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;
    auto res=socket.receive(msg);

    LOG_INFO("recv["<<msg.get(0)<<"]from["<<FLAGS_connect_endpoint<<"]");

    return res;
}
