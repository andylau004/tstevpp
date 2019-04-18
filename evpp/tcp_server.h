#pragma once

#include "evpp/inner_pre.h"
#include "evpp/event_loop.h"
#include "evpp/event_loop_thread_pool.h"
#include "evpp/tcp_callbacks.h"

#include "evpp/thread_dispatch_policy.h"
#include "evpp/server_status.h"

#include <map>

namespace evpp {

class Listener;


class EVPP_EXPORT TCPServer : public ThreadDispatchPolicy, public ServerStatus {
public:
    typedef std::function<void()> DoneCallback;

    // @brief The constructor of a TCPServer.
    // @param loop -
    // @param listen_addr - The listening address with "ip:port" format
    // @param name - The name of this object
    // @param thread_num - The working thread count
    TCPServer(EventLoop* l,
              const std::string& listen_addr,
              const std::string& name,
              uint32_t thread_num);
    ~TCPServer();

    // @brief Do the initialization works here.
    //  It will create a nonblocking TCP socket, and bind with the give address
    //  then listen on it. If there is anything wrong it will return false.
    // @return bool - True if anything goes well
    bool Init();

    // @brief Start the TCP server and we can accept new connections now.
    // @return bool - True if anything goes well
    bool Start();

    // @brief Stop the TCP server
    // @param cb - the callback cb will be invoked when
    //  the TCP server is totally stopped
    void Stop(DoneCallback cb = DoneCallback());

    // @brief Reinitialize some data fields after a fork
    void AfterFork();

public:
    // Set a connection event relative callback when the TCPServer
    // receives a new connection or an exist connection breaks down.
    // When these two events happened, the value of the parameter in the callback is:
    //      1. Received a new connection : TCPConn::IsConnected() == true
    //      2. An exist connection broken down : TCPConn::IsDisconnecting() == true
    void SetConnectionCallback(const ConnectionCallback& cb) {
        conn_fn_ = cb;
    }

    // Set the message callback to handle the messages from remote client
    void SetMessageCallback(MessageCallback cb) {
        msg_fn_ = cb;
    }
public:
    const std::string& listen_addr() const {
        return listen_addr_;
    }

private:
    void StopThreadPool();
    void StopInLoop(DoneCallback on_stopped_cb);
    void RemoveConnection(const TCPConnPtr& conn);
    void HandleNewConn(evpp_socket_t sockfd, const std::string& remote_addr/*ip:port*/, const struct sockaddr_in* raddr);
    EventLoop* GetNextLoop(const struct sockaddr_in* raddr);

private:
    EventLoop* loop_;  // the listening loop

    const std::string listen_addr_; // ip:port
    const std::string name_;

    std::unique_ptr<Listener> listener_;
    std::shared_ptr<EventLoopThreadPool> tpool_;
    ConnectionCallback conn_fn_;
    MessageCallback msg_fn_;

    DoneCallback stopped_cb_;

    // always in the listening loop thread
    uint64_t next_conn_id_ = 0;

    /* < the id of the connection, tcp connection ptr > */
    typedef std::map<uint64_t, TCPConnPtr> ConnectionMap;
    ConnectionMap connections_;
};


}//
