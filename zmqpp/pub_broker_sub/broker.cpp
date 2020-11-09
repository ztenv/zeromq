/**
 * @file broker.cpp
 * @brief broker demo
 * @author shlian
 * @version 1.0
 * @date 2020-11-06
 */
#include <gflags/gflags.h>

#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "../include/common.h"

DEFINE_string(front_endpoint,"tcp://*:15555","the endpoint of the broker that connected by sub client");
DEFINE_string(backend_endpoint,"tcp://*:15556","the endpoint of the broker that connected by pub server");
DEFINE_int32(io_thread_count,1,"the io_thread count of the zeromq context");

bool front_proxy(zmqpp::socket &xpub_socket,zmqpp::socket &xsub_scoket);
bool backend_proxy(zmqpp::socket &xsub_scoket,zmqpp::socket &xpub_socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    //bind front endpoint
    zmqpp::socket xpub_socket(context,zmqpp::socket_type::xpub);
    xpub_socket.set(zmqpp::socket_option::xpub_verbose,1);
    xpub_socket.bind(FLAGS_front_endpoint);

    //bind backend endpoint
    zmqpp::socket xsub_socket(context,zmqpp::socket_type::xsub);
    xsub_socket.bind(FLAGS_backend_endpoint);
    xsub_socket.send(std::string(1,0x01));

    //start event loop
    zmqpp::loop looper;
    looper.add(xsub_socket,std::bind(backend_proxy,std::ref(xsub_socket),std::ref(xpub_socket)));
    looper.add(xpub_socket,std::bind(front_proxy,std::ref(xpub_socket),std::ref(xsub_socket)));

    looper.start();

    return 0;
}

unsigned long long forward_topic=0;
bool front_proxy(zmqpp::socket &xpub_socket,zmqpp::socket &xsub_socket)
{
    zmqpp::message msg;

    bool res=xpub_socket.receive(msg);
    if(res)
    {
        ++forward_topic;
        //std::string topic=msg.get(0);//must manage the topics and process subscribe and unsubscribe topic,because socket receives only topic part and do not known if subscribe or unsubscribe

        LOG_INFO("handle subscribe topic:["<<msg.get(0)<<"],parts="<<msg.parts());
        std::string topic(1,0x01);
        topic.append(msg.get(0));
        res=xsub_socket.send(topic);
    }
    return res;
}

unsigned long long forward_data_msg=0;
bool backend_proxy(zmqpp::socket &xsub_socket,zmqpp::socket &xpub_socket)
{
    zmqpp::message msg;

    bool res=xsub_socket.receive(msg);
    if(res)
    {
        res=xpub_socket.send(msg);
        if((res)&&(forward_data_msg++%100==0))
        {
            LOG_INFO("forward:"<<forward_data_msg<<" data messages");
        }
    }
    return res;
}
