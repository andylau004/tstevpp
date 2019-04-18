// Modified from https://github.com/chenshuo/muduo/blob/master/examples/pingpong/client.cc

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "evpp/buffer.h"

//#include "evpp/tcp_client.h"
//#include "evpp/event_loop_thread_pool.h"
//#include "evpp/tcp_conn.h"

#include "evpp/server_status.h"
#include "evpp/fd_channel.h"

#include <event2/event.h>
#include <event2/event_struct.h>


//class Client;

//class Session {
//public:
//    Session(evpp::EventLoop* loop,
//            const std::string& serverAddr/*ip:port*/,
//            const std::string& name,
//            Client* owner)
//        : client_(loop, serverAddr, name),
//          owner_(owner),
//          bytes_read_(0),
//          bytes_written_(0),
//          messages_read_(0) {
//        client_.SetConnectionCallback(
//                    std::bind(&Session::OnConnection, this, std::placeholders::_1));
//        client_.SetMessageCallback(
//                    std::bind(&Session::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
//    }

//    void Start() {
//        client_.Connect();
//    }

//    void Stop() {
//        client_.Disconnect();
//    }

//    int64_t bytes_read() const {
//        return bytes_read_;
//    }

//    int64_t messages_read() const {
//        return messages_read_;
//    }

//private:
//    void OnConnection(const evpp::TCPConnPtr& conn);

//    void OnMessage(const evpp::TCPConnPtr& conn, evpp::Buffer* buf) {
//        LOG_TRACE << "bytes_read=" << bytes_read_ << " bytes_writen=" << bytes_written_;
//        ++messages_read_;
//        bytes_read_ += buf->size();
//        bytes_written_ += buf->size();
//        conn->Send(buf);
//    }

//private:
//    evpp::TCPClient client_;
//    Client* owner_;
//    int64_t bytes_read_;
//    int64_t bytes_written_;
//    int64_t messages_read_;
//};

//class Client {
//public:
//    Client(evpp::EventLoop* loop,
//           const std::string& serverAddr, // ip:port
//           int blockSize,
//           int sessionCount,
//           int timeout_sec,
//           int threadCount)
//        : loop_(loop),
//          session_count_(sessionCount),
//          timeout_(timeout_sec),
//          connected_count_(0) {
//        loop->RunAfter(evpp::Duration(double(timeout_sec)), std::bind(&Client::HandleTimeout, this));
//        tpool_.reset(new evpp::EventLoopThreadPool(loop, threadCount));
//        tpool_->Start(true);

//        for (int i = 0; i < blockSize; ++i) {
//            message_.push_back(static_cast<char>(i % 128));
//        }

//        for (int i = 0; i < sessionCount; ++i) {
//            char buf[32];
//            snprintf(buf, sizeof buf, "C%05d", i);
//            Session* session = new Session(tpool_->GetNextLoop(), serverAddr, buf, this);
//            session->Start();
//            sessions_.push_back(session);
//        }
//    }

//    ~Client() {
//    }

//    const std::string& message() const {
//        return message_;
//    }

//    void OnConnect() {
//        if (++connected_count_ == session_count_) {
//            LOG_WARN << "all connected";
//        }
//    }

//    void OnDisconnect(const evpp::TCPConnPtr& conn) {
//        if (--connected_count_ == 0) {
//            LOG_WARN << "all disconnected";

//            int64_t totalBytesRead = 0;
//            int64_t totalMessagesRead = 0;
//            for (auto &it : sessions_) {
//                totalBytesRead += it->bytes_read();
//                totalMessagesRead += it->messages_read();
//            }
//            LOG_WARN << totalBytesRead << " total bytes read";
//            LOG_WARN << totalMessagesRead << " total messages read";
//            LOG_WARN << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
//                     << " average message size";
//            LOG_WARN << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
//                     << " MiB/s throughput";
//            loop_->QueueInLoop(std::bind(&Client::Quit, this));
//        }
//    }

//private:
//    void Quit() {
//        tpool_->Stop();
//        loop_->Stop();
//        for (auto &it : sessions_) {
//            delete it;
//        }
//        sessions_.clear();
//        while (!tpool_->IsStopped() || !loop_->IsStopped()) {
//            std::this_thread::sleep_for(std::chrono::seconds(1));
//        }
//        tpool_.reset();
//    }

//    void HandleTimeout() {
//        LOG_WARN << "stop";
//        for (auto &it : sessions_) {
//            it->Stop();
//        }
//    }
//private:
//    evpp::EventLoop* loop_;
//    std::shared_ptr<evpp::EventLoopThreadPool> tpool_;
//    int session_count_;
//    int timeout_;
//    std::vector<Session*> sessions_;
//    std::string message_;
//    std::atomic<int> connected_count_;
//};

//void Session::OnConnection(const evpp::TCPConnPtr& conn) {
//    if (conn->IsConnected()) {
//        conn->SetTCPNoDelay(true);
//        conn->Send(owner_->message());
//        owner_->OnConnect();
//    } else {
//        owner_->OnDisconnect(conn);
//    }
//}



class CTestThread {
public:
    typedef std::function < int() > Functor;

    CTestThread();
    ~CTestThread();
public:
    bool Start(Functor pfnpre, Functor pfnpost = Functor());
    void Join();
private:
    void Run(const Functor& pre, const Functor& post);

private:
    std::shared_ptr<std::thread> thread_; // Guard by mutex_

};

CTestThread::CTestThread() {

}
CTestThread::~CTestThread() {

}
bool CTestThread::Start(Functor pfnpre, Functor pfnpost) {
    assert(thread_.get() == nullptr);
    thread_.reset( new std::thread(std::bind(&CTestThread::Run, this, pfnpre, pfnpost)) );

    if (pfnpre == nullptr) {
        LOG_WARN << "pfn pre is null" ;
    }
    if (pfnpost == nullptr) {
        LOG_WARN << "pfn post is null" ;
    }
    sleep(1);
    return true;
}
void CTestThread::Run(const Functor& pre, const Functor& post) {
//    if (name_.empty())
    {
        std::ostringstream os;
        os << "thread-" << std::this_thread::get_id();
        DLOG_TRACE << " Run : " << os.str();
//        name_ = os.str();
    }
    LOG_INFO << "tid=" << std::this_thread::get_id() << " execute pre functor.";
    auto fn = [this, pre]() {
        if (pre) {
            int iret = pre();
            LOG_INFO << "tid=" << std::this_thread::get_id() << " pre call, iret=" << iret;
        } else {
            LOG_INFO << "tid=" << std::this_thread::get_id() << " pre is null return ";
        }
    };
    fn();
    if (post) {
        LOG_INFO << "tid=" << std::this_thread::get_id() << " before post, post="/* << post->get()*/;
        post();
        LOG_INFO << "tid=" << std::this_thread::get_id() << " after post";
    }
}


void tstRun(const CTestThread::Functor& pre, const CTestThread::Functor& post) {
    LOG_INFO << "tid=" << std::this_thread::get_id() << " execute pre functor.";
//    auto fn = [pre]() {
        if (pre) {
            int iret = pre();
            LOG_INFO << "tid=" << std::this_thread::get_id() << " pre call, iret=" << iret;
        } else {
            LOG_INFO << "tid=" << std::this_thread::get_id() << " pre is null return ";
        }
//    };
//    fn();
    if (post) {
        LOG_INFO << "tid=" << std::this_thread::get_id() << " before post";
        post();
        LOG_INFO << "tid=" << std::this_thread::get_id() << " after post";
    } else {
        LOG_INFO << "tid=" << std::this_thread::get_id() << " post is null return ";
    }
}

void tst_2() {
//    std::shared_ptr<CTestThread> sp_obj;
//    sp_obj.reset(new CTestThread);

    std::shared_ptr<std::atomic<uint32_t>> started_count(new std::atomic<uint32_t>(0));
    std::shared_ptr<std::atomic<uint32_t>> exited_count(new std::atomic<uint32_t>(0));
    LOG_INFO << "tid=" << std::this_thread::get_id() << " started_count.use_count=" << started_count.use_count();
    LOG_INFO << "tid=" << std::this_thread::get_id() << " exited_count.use_count="  << exited_count.use_count();

    auto prepfn = [&started_count]() {
        LOG_INFO << "tid=" << std::this_thread::get_id() << ", prepfn start working..." << " started_count.use_count=" << started_count.use_count();
        return 1;
    };
    auto postpfn = [&exited_count]() {
        LOG_INFO << "tid=" << std::this_thread::get_id() << ", postpfn start working..." << " exited_count.use_count=" << exited_count.use_count();
        return 1;
    };//LOG_INFO << "tst_2   tid=" << std::this_thread::get_id();

    tstRun( prepfn, postpfn );
//    sp_obj->Start(prepfn, postpfn);
//    getchar();
}

void tst_1() {
    LOG_INFO << "maithread";
    evpp::ServerStatus ctss;
    ctss.SetInitStatus();
    LOG_INFO << "ctss.StatusToString=" << ctss.StatusToString();

    getchar();
}

int main(int argc, char* argv[]) {

    LOG_INFO << "sizeof(struct event)=" << sizeof(struct event);
    LOG_INFO << "sizeof(event)=" << sizeof(event);
    return 1;

    tst_2(); return 1;
    tst_1(); return 1;

//    if (argc != 7) {
//        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> <sessions> <time_seconds>\n");
//        return -1;
//    }

//    const char* ip = argv[1];
//    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
//    int threadCount = atoi(argv[3]);
//    int blockSize = atoi(argv[4]);
//    int sessionCount = atoi(argv[5]);
//    int timeout = atoi(argv[6]);

//    evpp::EventLoop loop;
//    std::string serverAddr = std::string(ip) + ":" + std::to_string(port);

//    Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
//    loop.Run();
    return 0;
}


#ifdef WIN32
#include "../winmain-inl.h"
#endif



