#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <exception>
#include <mutex>
#include <functional>

#include <boost/bind/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/placeholders.hpp>

#include <zmq.h>
#include <zmqpp/context.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>


#include <gflags/gflags.h>

using namespace std;

DEFINE_string(id,"sub_client1","app id");
DEFINE_string(endpoint,"tcp://127.0.0.1:6666","connect endpoint");
DEFINE_string(topics,"sub","topics splited by , such as:pub,pub2,pub3");
DEFINE_int32(send_interval,1000,"send msg interval,ms");
DEFINE_string(socket_type,"xsub","socket types,must be xsub or sub");

#define USE_LOOP
//#define USE_BOOST

#ifdef USE_BOOST
void send_msg(boost::asio::steady_timer &timer,zmqpp::socket *socket);
void recv_msg(boost::asio::io_service::strand &strand,zmqpp::socket *socket);
#endif

#ifdef USE_LOOP
bool loop_recv_msg(zmqpp::socket *socket);
bool loop_send_msg(zmqpp::socket *socket);
#endif

vector<string> split(const string &str,const string &pattern)
{
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }
    delete[] strc;
    return resultVec;
}

//atomic<bool> flag(true);

int main (int argc,char *argv[])
{
    google::SetUsageMessage("usage:");
    google::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;
    zmqpp::socket_type st=FLAGS_socket_type=="xsub"?zmqpp::socket_type::xsub:zmqpp::socket_type::sub;
    zmqpp::socket xsub_socket(context,st);
    xsub_socket.connect(FLAGS_endpoint);

    //string msg(1,0x1);//subscribe all topics and will recv all msg
    //xsub_socket.send(msg);

    //subscribe topics
    if (FLAGS_topics.find(","))
    {
        vector<string> topic=split(FLAGS_topics,",");
        for(auto &item:topic)
        {
            string msg(1,0x1);
            msg.append(item);
            //xsub_socket.send(msg);

            zmqpp::message mmsg;
            mmsg.add(msg.data());//the code is same as above's msg(1,0x1) and append(item);

            if(st==zmqpp::socket_type::xsub)
            {
                xsub_socket.send(mmsg);
            }else{
                xsub_socket.subscribe(item);
            }
        }
    }else{
        string msg(1,0x1);
        msg.append(FLAGS_topics);
        if(st==zmqpp::socket_type::xsub)
        {
            xsub_socket.send(msg);
        }else{
            xsub_socket.subscribe(msg);
        }
    }

#ifdef USE_LOOP
    zmqpp::loop looper;
    looper.add(xsub_socket,std::bind(loop_recv_msg,&xsub_socket),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    if(st==zmqpp::socket_type::xsub)
    {
        looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(loop_send_msg,&xsub_socket));
    }else{
        cout<<"The socket type is sub,so it can not send msg and will not run send msg loop."<<endl;
    }
    looper.start();
#endif

#ifdef USE_BOOST
    boost::asio::io_service ios;
    boost::asio::io_service::work worker(ios);
    boost::asio::io_service::strand strand(ios);

    //start send and recv tasks
    boost::asio::steady_timer timer(ios);
    timer.expires_from_now(std::chrono::seconds(1));
    timer.async_wait(boost::bind(send_msg,boost::ref(timer),&xsub_socket));
    strand.post(std::bind(recv_msg,boost::ref(strand),&xsub_socket));
    thread t1=thread(boost::bind(&boost::asio::io_service::run,boost::ref(ios)));
    ios.run();
    t1.join();
#endif
    xsub_socket.close();
    return 0;
}

#ifdef USE_BOOST
void send_msg(boost::asio::steady_timer &timer,zmqpp::socket *socket)//,boost::system::error_code &ec)
{
    //while(flag)
    {
        auto time=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
        ss<<std::put_time(std::localtime(&time),"%Y%m%d%H:%M:%S");
        string msg(FLAGS_id);
        msg.append(":");
        msg.append(ss.str());

        bool res=false;
        {
            res=socket->send(msg);
        }

        cout<<"send:["<<msg<<"],res="<<res<<endl;
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    timer.expires_from_now(std::chrono::seconds(1));
    timer.async_wait(boost::bind(send_msg,boost::ref(timer),socket));//,boost::asio::placeholders::error));
}

void recv_msg(boost::asio::io_service::strand &strand,zmqpp::socket *socket)
{
    std::string msg;
    zmqpp::poller poll;//this is a demo only,so let it go
    poll.add(*socket,zmqpp::poller::poll_in|zmqpp::poller::poll_error);

    bool res=false;
    if(poll.poll(10))
    {
        auto e=poll.events(*socket);//check events
        if(e==zmqpp::poller::poll_in)
        {
            res=socket->receive(msg);
            cout<<"recv:["<<msg<<"],res="<<res<<endl;
        }else if (e==zmqpp::poller::poll_error)
        {
            cout<<"zmq poll error"<<endl;
        }
    }
    strand.post(boost::bind(recv_msg,boost::ref(strand),socket));
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

    bool res=false;
    {
        res=socket->send(msg);
    }
    cout<<"send:["<<msg<<"],res="<<res<<endl;
    return true;
}

bool loop_recv_msg(zmqpp::socket *socket)
{
    std::string msg;
    auto res=socket->receive(msg);
    cout<<"recv:["<<msg<<"],res="<<res<<endl;
    return true;
}
#endif
