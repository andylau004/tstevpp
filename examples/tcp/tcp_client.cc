

#include <evpp/tcp_client.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>



int main(int argc, char* argv[]) {
//    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();

    std::string addr = "127.0.0.1:9099";

    evpp::EventLoop loop;
    evpp::TCPClient client(&loop, addr, "TCPPingPongClient");

    auto pfn_msgCb = [&loop, &client](const evpp::TCPConnPtr& conn, evpp::Buffer* msg) {
        LOG_TRACE << "Receive a message [" << msg->ToString() << "]";
        client.Disconnect();
    };
    client.SetMessageCallback(pfn_msgCb);

    auto pfn_cnnCb = [](const evpp::TCPConnPtr& conn) {
        if (conn->IsConnected()) {
            LOG_INFO << "Connected to " << conn->remote_addr();
            conn->Send("hello");
        } else {
            LOG_INFO << "Disconnect...";
            conn->loop()->Stop();
        }
    };
    client.SetConnectionCallback(pfn_cnnCb);
    client.Connect();
    loop.Run();
    return 1;
}
