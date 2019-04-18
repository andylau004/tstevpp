

#define GOOGLE_STRIP_LOG 1

#include <evpp/tcp_server.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>

#include "evpp/logging.h"


#include <glog/logging.h>
#include <glog/raw_logging.h>



class GLogHelper
{
public:
    GLogHelper(char* program) {
        ::google::InitGoogleLogging(program);
        FLAGS_log_dir = ".";
        FLAGS_stderrthreshold = google::GLOG_INFO;

        FLAGS_logtostderr = false;      //log messages go to stderr instead of logfiles
        FLAGS_alsologtostderr = true;    //log messages go to stderr in addition to logfiles
        FLAGS_colorlogtostderr = true;    //color messages logged to stderr (if supported by terminal)
        FLAGS_v = 255;
//        FLAGS_v = 3;
    }
    ~GLogHelper() {
        google::ShutdownGoogleLogging();
    }
};

int main(int argc, char* argv[]) {
    GLogHelper aaa("tcpsrv.log");

    std::string addr = "0.0.0.0:9099";
    int thread_num = 1;

    evpp::EventLoop loop;
    evpp::TCPServer server(&loop, addr, "TCPEchoServer", thread_num);

    auto msgCb = [](const evpp::TCPConnPtr& conn, evpp::Buffer* msg) {
        conn->Send(msg);
    };
    server.SetMessageCallback( msgCb );

    auto cnnCb = [](const evpp::TCPConnPtr& newCnn) {
        if (newCnn->IsConnected()) {
            LOG_INFO << "usecount=" << newCnn.use_count() << " -- A new connection from " << newCnn->remote_addr();
        } else {
            LOG_INFO << "usecount=" << newCnn.use_count() << " -- Lost the connection from " << newCnn->remote_addr();
        }
    };
    server.SetConnectionCallback( cnnCb );

    server.Init();
    server.Start();
    loop.Run();
    return 1;
}
