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

void build_reply(std::vector<char*> msg, void *socket, const char * status_code = "200", const char * status_text = "ok");

int main()
{

    void *context = zmq_ctx_new();
    void * responser = zmq_socket(context, ZMQ_REP);

    std::thread t = std::thread(std::bind(auth_server, context));

    int flag = 1;
    int rc = zmq_setsockopt(responser, ZMQ_PLAIN_SERVER, &flag, sizeof(flag));
    cout << "rc = " << rc << endl;

    rc = zmq_bind(responser, "tcp://127.0.0.1:55555");
    cout << "rc = " << rc << endl;

    cout << "start to work" << endl;
    while(true) {
        std::array<char, 1024> buff;
        memset(buff.data(), 0, buff.max_size());

        auto res = zmq_recv(responser, buff.data(), buff.max_size(), 0);
        cout << "recv:" << buff.data() <<endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        res = zmq_send(responser, buff.data(), res, 0);
    }

    t.join();
    return 0;
}

void auth_server(void *context) {
    
    void * auth_socket = zmq_socket(context, ZMQ_ROUTER);

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

            char * version = msg[2];
            cout << "version:" << version << endl;

            char * sequence = msg[3];
            cout << "sequence:" << sequence << endl;

            char * domain = msg[4];
            cout << "domain:" << domain << endl;

            char * address = msg[5];
            cout << "address:" << address << endl;

            char * identity = msg[6];
            cout << "identity:" << identity << endl;

            char * mechanism = msg[7];
            cout << "mechanism:" << mechanism << endl;

            char * user = msg[8];
            cout << "user:" << user << endl;

            char * pwd = msg[9];
            cout << "pwd:" << pwd << endl;

            build_reply(msg, auth_socket);

            for_each(msg.begin(), msg.end(), [&](char * ele ) { delete [] ele ;});
            msg.clear();
        }
    }
}

void build_reply(std::vector<char*> msg, void *socket, const char * status_code, const char * status_text) {
    zmq_msg_t p0;
    int rc = zmq_msg_init_size(&p0, 5);
    memcpy(zmq_msg_data(&p0), msg[0], 5);       
    rc = zmq_msg_send(&p0, socket, ZMQ_SNDMORE);  // the dealer id
    cout << "send res:" << rc << endl;

    zmq_msg_t p1;
    rc = zmq_msg_init_size(&p1, 0);
    rc = zmq_msg_send(&p1, socket, ZMQ_SNDMORE);  //zero frame
    cout << "send res:" << rc << endl;

    zmq_msg_t p2;
    rc = zmq_msg_init_size(&p2, strlen(msg[2]));
    memcpy(zmq_msg_data(&p2), msg[2], strlen(msg[2])); // version
    rc = zmq_msg_send(&p2, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p3;
    rc = zmq_msg_init_size(&p3, strlen(msg[3]));
    memcpy(zmq_msg_data(&p3), msg[3], strlen(msg[3])); //sequence
    rc = zmq_msg_send(&p3, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p4;
    //const char * status_code = "200";
    rc = zmq_msg_init_size(&p4, strlen(status_code));
    memcpy(zmq_msg_data(&p4), status_code, strlen(status_code)); //status_code
    rc = zmq_msg_send(&p4, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p5;
    //const char * status_text = "ok";
    rc = zmq_msg_init_size(&p5, strlen(status_text));
    memcpy(zmq_msg_data(&p5), status_code, strlen(status_text)); //status_text
    rc = zmq_msg_send(&p5, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p6;
    rc = zmq_msg_init_size(&p6, strlen(msg[8]));
    memcpy(zmq_msg_data(&p6), msg[8], strlen(msg[8])); //user_id/user_name
    rc = zmq_msg_send(&p6, socket, ZMQ_SNDMORE);
    cout << "send res:" << rc << endl;

    zmq_msg_t p7;
    rc = zmq_msg_init_size(&p7, 0);
    rc = zmq_msg_send(&p7, socket, 0);  //zero frame
    cout << "send res:" << rc << endl;

}

