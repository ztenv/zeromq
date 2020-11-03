#include <ctime>

#include <iostream>
#include <string>

#include <gflags/gflags.h>

#include <zmqpp/auth.hpp>
#include <zmqpp/context.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

using namespace std;

DEFINE_string(id,"server","app id");
DEFINE_string(endpoint,"tcp://*:11111","endpoint to bind");
DEFINE_string(user_name,"","plain user name");
DEFINE_string(password,"","plain password");

bool recv_msg(zmqpp::socket &socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    zmqpp::auth author(context);
    if((FLAGS_user_name.length()>0)&&(FLAGS_password.length()>0))
    {
        author.configure_plain(FLAGS_user_name,FLAGS_password);
        author.set_verbose(true);
    }

    zmqpp::socket router(context,zmqpp::socket_type::router);
    if((FLAGS_user_name.length()>0)&&(FLAGS_password.length()>0))
    {
        router.set(zmqpp::socket_option::plain_server,1);
    }

    router.bind(FLAGS_endpoint);

    zmqpp::loop looper;
    looper.add(router,std::bind(recv_msg,std::ref(router)),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}


bool recv_msg(zmqpp::socket &socket)
{
    zmqpp::message msg;
    bool res=socket.receive(msg);

    cout<<FLAGS_id<<" recv:"<<msg.get(1)<<endl;;
    return res;
}
