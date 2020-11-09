/**
 * @file sub.cpp
 * @brief sub demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-06
 */

#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(broker_endpoint,"tcp://127.0.0.1:15555","the broker endpoint that connected by sub client");
DEFINE_string(topic,"","the subscribing topics");
DEFINE_int32(io_thread_count,1,"the io_thread count of the zeromq context");

bool handle_message(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    zmqpp::socket sub_socket(context,zmqpp::socket_type::sub);
    sub_socket.connect(FLAGS_broker_endpoint);
    sub_socket.subscribe(FLAGS_topic);
    LOG_INFO("subscribe:"<<FLAGS_topic);

    zmqpp::loop looper;
    looper.add(sub_socket,std::bind(handle_message,std::ref(sub_socket)));
    looper.start();
    
    return 0;
}

bool handle_message(zmqpp::socket &socket)
{
    zmqpp::message msg;

    auto res=socket.receive(msg);
    if(res)
    {
        LOG_INFO("recv ["<<msg.get(0)<<"]"<<"["<<msg.get(1)<<"]");
    }
    return res;
}

