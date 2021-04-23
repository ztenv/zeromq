#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <thread>

#include <cstring>

#include <zmq.h>

using namespace std;

int main()
{
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    const char *name = "admin";
    const char *pwd = "pwd";

    auto rc = zmq_setsockopt(requester, ZMQ_PLAIN_USERNAME, name, strlen(name));
    cout << "rc=" << rc <<endl;
    rc = zmq_setsockopt(requester, ZMQ_PLAIN_PASSWORD, pwd, strlen(pwd));
    cout << "rc=" << rc <<endl;

    const char *id = "auth_request";
    rc = zmq_setsockopt(requester, ZMQ_ROUTING_ID, id, strlen(id));
    cout << "rc=" << rc <<endl;

    rc = zmq_connect(requester, "tcp://127.0.0.1:55555");
    cout << "rc=" << rc <<endl;

    cout << "start to send msg" <<endl;
    for(int i = 0; i<1000000000; ++i) {
        ostringstream oss;
        oss << i;
        auto res = zmq_send(requester, oss.str().c_str(), oss.str().length(), 0);
        cout << "send:" << oss.str() << endl;

        std::array<char, 1024> buf;
        memset(buf.data(), 0, buf.max_size());
        res = zmq_recv(requester, buf.data(), buf.max_size(), 0);

        cout << "recv:" << buf.data() <<endl;
    }
    return 0;
}
