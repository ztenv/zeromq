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

void build_reply(std::vector<char*> msg, void *socket);

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

            build_reply(msg, auth_socket);

            for_each(msg.begin(), msg.end(), [&](char * ele ) { delete [] ele ;});
            msg.clear();
        }
    }
}

void build_reply(std::vector<char*> msg, void *socket) {
    //zmq_msg_t p1;
    //int rc = zmq_msg_init_size(&p1, 5);
    //memcpy(zmq_msg_data(&p1), msg[0], 5);

    //rc = zmq_msg_send(&p1, socket, ZMQ_SNDMORE);

    int rc = 0;

    zmq_msg_t p2;
    rc = zmq_msg_init_size(&p2, strlen(msg[0]));
    memcpy(zmq_msg_data(&p2), msg[0], strlen(msg[0]));
    rc = zmq_msg_send(&p2, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p3;
    rc = zmq_msg_init_size(&p3, strlen(msg[1]));
    memcpy(zmq_msg_data(&p3), msg[1], strlen(msg[1]));
    rc = zmq_msg_send(&p3, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p4;
    const char * status_code = "200";
    rc = zmq_msg_init_size(&p4, strlen(status_code));
    memcpy(zmq_msg_data(&p4), status_code, strlen(status_code));
    rc = zmq_msg_send(&p4, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p5;
    const char * status_text = "ok";
    rc = zmq_msg_init_size(&p5, strlen(status_text));
    memcpy(zmq_msg_data(&p5), status_code, strlen(status_text));
    rc = zmq_msg_send(&p5, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p6;
    rc = zmq_msg_init_size(&p6, strlen(msg[6]));
    memcpy(zmq_msg_data(&p6), msg[6], strlen(msg[6]));
    rc = zmq_msg_send(&p6, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p7;
    rc = zmq_msg_init_size(&p7, 0);
    rc = zmq_msg_send(&p7, socket, 0);
    cout << "send res:" << rc << endl;

}

