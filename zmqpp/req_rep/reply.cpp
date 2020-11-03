#include <chrono>
#include <iostream>
#include <functional>

#include <gflags/gflags.h>
#include <zmqpp/context.hpp>
#include <zmqpp/context_options.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include "common.h"

using namespace std;

DEFINE_string(endpoint,"tcp://*:55555","the server endpoint that connected by the client");
DEFINE_string(id,"rep_socket","the socket id");

DEFINE_int32(io_thread_count,1,"the io thread count of zmq io_context");

bool receive_msg(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("Usage");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    //build zmq context
    zmqpp::context context;
    context.set(zmqpp::context_option::io_threads,FLAGS_io_thread_count);

    //build zmq socket
    zmqpp::socket rep_socket(context,zmqpp::socket_type::rep);
    rep_socket.set(zmqpp::socket_option::identity,FLAGS_id);
    rep_socket.bind(FLAGS_endpoint);
    cout<<"connected to "<<FLAGS_endpoint<<endl;

    //build the loop event
    zmqpp::loop looper;
    //register io events:poll_in and poll_error
    looper.add(rep_socket,std::bind(receive_msg,std::ref(rep_socket)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool receive_msg(zmqpp::socket &socket)
{
    bool res=true;
    zmqpp::message msg;

    res=socket.receive(msg);
    if(res)
    {
        LOG_INFO("recv ["<<msg.get(1)<<"]from["<<msg.get(0)<<"]");

        zmqpp::message rep_msg;
        rep_msg.add(FLAGS_id); //the first frame
        rep_msg.add("<"+FLAGS_id+">"+msg.get<std::string>(1)); //the second frame
        res=socket.send(rep_msg);
        if(res)
        {
            LOG_INFO("reply ["<<msg.get(1)<<"]to["<<msg.get(0)<<"]");
        }else{
            LOG_ERROR("reply error");
        }
    }else{
        LOG_ERROR("recv msg error,id="<<FLAGS_id);
    }

    return res;
}

