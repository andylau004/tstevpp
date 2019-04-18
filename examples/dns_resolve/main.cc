#include <evpp/event_loop.h>
#include <evpp/dns_resolver.h>
#include <evpp/sockets.h>

#include "evpp/logging.h"


#ifdef WIN32
#include "examples/winmain-inl.h"
#endif

int main(int argc, char* argv[]) {
    std::string host = "www.so.com";
    if (argc > 1) {
        host = argv[1];
    }
    DLOG_TRACE << " work process begin";

    evpp::EventLoop loop;
    auto fn_resolved = [&loop, &host](const std::vector <struct in_addr>& addrs) {
        DLOG_TRACE << "Entering fn_resolved";
        for (auto addr : addrs) {
            struct sockaddr_in saddr;
            memset(&saddr, 0, sizeof(saddr));
            saddr.sin_addr = addr;
            DLOG_TRACE << "DNS resolved host=" << host << " ip=" << evpp::sock::ToIP(evpp::sock::sockaddr_cast(&saddr));
        }

        loop.RunAfter(evpp::Duration(6.5), [&loop]() { loop.Stop(); });
    };

    std::shared_ptr<evpp::DNSResolver> dns_resolver(new evpp::DNSResolver(&loop, host, evpp::Duration(7.0/*1.0*/), fn_resolved));
    dns_resolver->Start();
    loop.Run();

    return 0;
}
