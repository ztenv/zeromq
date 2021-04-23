#include <iostream>
#include <chrono>
#include <string>
#include <iostream>
#include <array>
#include <cstring>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>


#include <thread>
#include <zmq.h>

using namespace std;

void auth_server(void *context);

int main()
{

    void *context = zmq_ctx_new();
    void * responser = zmq_socket(context, ZMQ_REP);

    std::thread t = std::thread(std::bind(auth_server, context));

    int flag = 1;
    int rc = zmq_setsockopt(responser, ZMQ_PLAIN_SERVER, &flag, sizeof(flag));
    cout << "rc = " << rc << endl;
    //const char *name = "admin";
    //const char *pwd = "pwd";
    //rc = zmq_setsockopt(responser, ZMQ_PLAIN_USERNAME, name, strlen(name));
    //cout << "rc = " << rc << endl;
    //rc = zmq_setsockopt(responser, ZMQ_PLAIN_PASSWORD, pwd, strlen(pwd));
    //cout << "rc = " << rc << endl;
    rc = zmq_bind(responser, "tcp://127.0.0.1:55555");
    cout << "rc = " << rc << endl;

    cout << "start to work" << endl;
    while(true) {
        std::array<char, 1024> buffer;

        auto res = zmq_recv(responser, buffer.data(), buffer.max_size(), 0);
        cout << "recv:" << buffer.data() <<endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        res = zmq_send(responser, buffer.data(), res, 0);
    }

    t.join();
    return 0;
}

void auth_server(void *context) {
    
    void * auth_socket = zmq_socket(context, ZMQ_REP);

    auto rc = zmq_bind(auth_socket, "inproc://zeromq.zap.01");
    cout << "auth_socket zmq_bind:" << rc << endl;
    std::vector<char *> msg;

    while(true) {
        zmq_msg_t  frame;
        zmq_msg_init(&frame);
        auto res = zmq_msg_recv(&frame, auth_socket, 0);

        if(res >= 0 ) {
            cout << "auth_socket recv, lenth = " << res <<endl;
            char * data = new char[res+1];
            data[res] = 0;
            if (res > 0) {
                memcpy(data, zmq_msg_data(&frame), res);
            }
            msg.push_back(data);
            cout << data <<endl;
            zmq_msg_close(&frame);
        }

        if(zmq_msg_more(&frame)) {
            continue;
        } else {
            cout << "frame size:" << msg.size() << endl;

            char * version = msg[0];
            cout << "version:" << version << endl;

            char * sequence = msg[1];
            cout << "sequence:" << sequence << endl;

            char * domain = msg[2];
            cout << "domain:" << domain << endl;

            char * address = msg[3];
            cout << "address:" << address << endl;

            char * identity = msg[4];
            cout << "identity:" << identity << endl;

            char * mechanism = msg[5];
            cout << "mechanism:" << mechanism << endl;

            char * user = msg[6];
            cout << "user:" << user << endl;

            char * pwd = msg[7];
            cout << "pwd:" << pwd << endl;

            for_each(msg.begin(), msg.end(), [&](char * ele ) { delete [] ele ;});
            msg.clear();
            break;
        }
    }
}

