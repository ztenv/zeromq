#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <exception>

#include <zmq.h>
#include <zmqpp/context.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include <boost/bind/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <gflags/gflags.h>

using namespace std;

DEFINE_string(id,"pub_server","app id");
DEFINE_string(endpoint,"tcp://127.0.0.1:5555","connect endpoint");
DEFINE_int32(send_interval,1000,"send interval,ms");

//#define USE_BOOST
#define USE_LOOP

#ifdef USE_BOOST
void send_msg(boost::asio::steady_timer &timer,zmqpp::socket *socket);
void recv_msg(boost::asio::io_service::strand &strand,zmqpp::socket *socket);
#endif

#ifdef USE_LOOP
bool loop_send_msg(zmqpp::socket *socket);
bool loop_recv_msg(zmqpp::socket *socket);
#endif

atomic<bool> flag(true);
int main (int argc,char *argv[])
{
    google::SetUsageMessage("usage:");
    google::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    zmqpp::socket xpub_socket(context,zmqpp::socket_type::xpub);
    xpub_socket.connect(FLAGS_endpoint);

#ifdef USE_LOOP
    zmqpp::loop looper;
    looper.add(xpub_socket,std::bind(loop_recv_msg,&xpub_socket),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(loop_send_msg,&xpub_socket));
    looper.start();
#endif

#ifdef USE_BOOST
    boost::asio::io_service ios;
    boost::asio::io_service::work worker(ios);
    boost::asio::io_service::strand strand(ios);
    //start send and recv tasks
    boost::asio::steady_timer timer(ios);
    timer.expires_from_now(std::chrono::milliseconds(100));
    timer.async_wait(boost::bind(send_msg,boost::ref(timer),&xpub_socket));
    strand.post(std::bind(recv_msg,boost::ref(strand),&xpub_socket));
    thread t1=thread(boost::bind(&boost::asio::io_service::run,boost::ref(ios)));
    ios.run();
    t1.join();
#endif

    xpub_socket.close();

    return 0;
}

#ifdef USE_BOOST
void send_msg(boost::asio::steady_timer &timer,zmqpp::socket *socket)
{
    auto time=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&time),"%Y-%m-%d %H:%M:%S");
    string msg(FLAGS_id);
    msg.append(":");
    msg.append(ss.str());
    bool res=socket->send(msg);
    cout<<"pub:["<<msg<<"],res="<<res<<endl;
    timer.expires_from_now(std::chrono::milliseconds(100));
    timer.async_wait(boost::bind(send_msg,boost::ref(timer),socket));
}

void recv_msg(boost::asio::io_service::strand &strand,zmqpp::socket *socket)
{
    zmqpp::poller poll;//it is a demo only ,so let it go
    poll.add(*socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);

    std::string msg;
    //zmqpp::message_t msg;
    if(poll.poll(10))
    {
        auto e=poll.events(*socket);
        if(e==zmqpp::poller::poll_in)
        {
            bool res=socket->receive(msg);
            cout<<"recv:["<<msg<<"],res="<<res<<endl;
        }else if (e==zmqpp::poller::poll_error)
        {
            cout<<"poll error"<<endl;
        }
    }
    strand.post(std::bind(recv_msg,boost::ref(strand),socket));
}
#endif

#ifdef USE_LOOP
bool loop_send_msg(zmqpp::socket *socket)
{
    auto time=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&time),"%Y-%m-%d %H:%M:%S");
    string msg(FLAGS_id);
    msg.append(":");
    msg.append(ss.str());
    bool res=socket->send(msg);
    cout<<"pub:["<<msg<<"],res="<<res<<endl;
    return true;
}

bool loop_recv_msg(zmqpp::socket *socket)
{
    std::string msg;
    bool res=socket->receive(msg);
    cout<<"recv:["<<msg<<"],res="<<res<<endl;
    return true;
}
#endif
