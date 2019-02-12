#pragma once

#include <atomic>

#include "evpp/inner_pre.h"
#include "evpp/buffer.h"
#include "evpp/tcp_callbacks.h"
#include "evpp/slice.h"
#include "evpp/any.h"
#include "evpp/duration.h"

namespace evpp {

class EventLoop;
class FdChannel;
class TCPClient;
class InvokeTimer;


class EVPP_EXPORT TCPConn : public std::enable_shared_from_this<TCPConn> {
public:
    enum Type {
        kIncoming = 0, // The type of a TCPConn held by a TCPServer
        kOutgoing = 1, // The type of a TCPConn held by a TCPClient
    };
    enum Status {
        kDisconnected = 0,
        kConnecting = 1,
        kConnected = 2,
        kDisconnecting = 3,
    };
public:
    TCPConn(EventLoop* l,
            const std::string& name,
            evpp_socket_t sockfd,
            const std::string& laddr,
            const std::string& raddr,
            uint64_t id);
    ~TCPConn();

    void Close();
    void Send(const char* s) {
        Send(s, strlen(s));
    }
    void Send(const void* d, size_t dlen);
    void Send(const std::string& d);
    void Send(const Slice& message);
    void Send(Buffer* buf);
public:
    EventLoop* loop() const {
        return loop_;
    }
    evpp_socket_t fd() const {
        return fd_;
    }
    uint64_t id() const {
        return id_;
    }


private:
    EventLoop* loop_;
    int  fd_;
    uint64_t id_ = 0;

    std::string name_;
    std::string local_addr_; // the local address with form : "ip:port"
    std::string remote_addr_; // the remote address with form : "ip:port"

    std::unique_ptr<FdChannel> chan_;

    Buffer input_buffer_;
    Buffer output_buffer_;

    enum { kContextCount = 16, };
    Any context_[kContextCount];

    Type type_;
    std::atomic<Status> status_;

    size_t high_water_mark_ = 128 * 1024 * 1024; // Default 128MB

    // The delay time to close a incoming connection which has been shutdown by peer normally.
    // Default is 0 second which means we disable this feature by default.
    Duration close_delay_ = Duration(0.0);
    std::shared_ptr<InvokeTimer> delay_close_timer_; // The timer to delay close this TCPConn

    ConnectionCallback conn_fn_; // This will be called to the user application layer
    MessageCallback msg_fn_;     // This will be called to the user application layer
    WriteCompleteCallback write_complete_fn_;  // This will be called to the user application layer
    HighWaterMarkCallback high_water_mark_fn_; // This will be called to the user application layer
    CloseCallback close_fn_; // This will be called to TCPClient or TCPServer

};



}
