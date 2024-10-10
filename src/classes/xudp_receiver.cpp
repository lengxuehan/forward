#include <netdb.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <atomic>
#include <cstdlib>

#include "xudp.h"
#include "xudp_receiver.h"
#include "storager_mgr.h"
#include "structs/pack_helper.h"
#include "iguana/iguana.hpp"

namespace forward {
namespace classes {

    std::atomic_bool g_loop{true};

    static void handle_recv_msg(char* p, uint32_t size) {
        auto& mgr = StorageMgr::get_instance();
        const Cmd *cmd = (const Cmd *)p;
        if(cmd->no == 1) {
            StructA struct_a;
            iguana::from_pb(struct_a, std::string_view(cmd->data, cmd->len - sizeof(Cmd)));

            //printf("total id : %lu\n", struct_a.ns);
            //printf("total id : %lu\n", struct_a.total_id);
            //printf("total id : %lu\n", struct_a.data_id);

            struct_a.recv_ns = mgr.get_ns();
            mgr.get_storager("StructA")->asyncWrite(struct_a);
        } else if(cmd->no == 2) {
            StructB struct_b;
            iguana::from_pb(struct_b, std::string_view(cmd->data, cmd->len - sizeof(Cmd)));
            struct_b.recv_ns = mgr.get_ns();
            mgr.get_storager("StructB")->asyncWrite(struct_b);
        }
    }

    struct connect{
        xudp *x;
        xudp_channel *ch;
        void (*handler)(struct connect *);
    };

    void handler(struct connect *c)
    {
        xudp_msg *m;
        xudp_channel *ch = c->ch;
        int n, i, ret;

        xudp_def_msg(hdr, 100);

        while (true) {
            hdr->used = 0;

            n = xudp_recv_channel(ch, hdr, 0);
            if (n < 0)
                break;

            for (i = 0; i < hdr->used; ++i) {
                m = hdr->msg + i;
                //printf("recv msg: %.*s", m->size, m->p);
                printf("recv msg: %d\n", m->size);
                handle_recv_msg(m->p, m->size);
            }

            xudp_recycle(hdr);
            xudp_commit_channel(ch);
        }
    }

    static int epoll_add(xudp *x, int efd)
    {
        struct epoll_event e;
        struct connect *c;
        xudp_channel *ch;
        xudp_group *g;
        int fd;

        e.events = EPOLLIN;

        g = xudp_group_get(x, 0);

        xudp_group_channel_foreach(ch, g) {

            fd = xudp_channel_get_fd(ch);

            c = (struct connect*)(malloc(sizeof(*c)));
            c->ch = ch;
            c->x = x;
            c->handler = handler;

            e.data.ptr = c;

            epoll_ctl(efd, EPOLL_CTL_ADD, fd, &e);
        }

        return 0;
    }

    static int loop(int efd)
    {
        struct connect *c;
        struct epoll_event e[1024];
        int n, i;

        while (g_loop) {
            n = epoll_wait(efd, e, sizeof(e)/sizeof(e[0]), -1);

            if (n == 0)
                continue;

            if (n < 0) {
                //printf("epoll wait error: %s\n", strerror(errno));
                continue;
            }

            for (i = 0; i < n; ++i) {
                c = (struct connect*)(e[i].data.ptr);
                c->handler(c);
            }
        }
        return 0;
    }

    XUdpReceiver::XUdpReceiver(const structs::ReceiverChannel& channel)
            : channel_(channel){
    }

    void XUdpReceiver::initialize() {
        int ret, size;
        std::cout << "XUdpReceiver::initialize with ip:" << channel_.str_ip_
            << " port:" << channel_.port_ << std::endl;
        ret = getaddrinfo(channel_.str_ip_.c_str(),
                              std::to_string(channel_.port_).c_str(), NULL, &addr_info_);
        if (ret) {
            printf("getaddrinfo err.");
            return;
        }

        const char* command = "ip link set eth0 xdp off";
        ret = system(command);
        if (ret == 0) {
            std::cout << command <<" exec successful" << std::endl;
        } else {
            std::cout << command <<" exec failed" << std::endl;
            return;
        }

        xudp_conf conf = {};
        conf.group_num     = 1;
        conf.log_with_time = true;
        conf.log_level = XUDP_LOG_WARN;

        x_ = xudp_init(&conf, sizeof(conf));
        if (!x_) {
            printf("xudp init fail\n");
            return;
        }

        if (addr_info_->ai_family == AF_INET) {
            printf("AF_INET addr.\n");
            size = sizeof(struct sockaddr_in);
        } else {
            printf("AF_INET6 addr.\n");
            size = sizeof(struct sockaddr_in6);
        }

        ret = xudp_bind(x_, (struct sockaddr *)addr_info_->ai_addr, size, 1);
        if (ret) {
            xudp_free(x_);
            printf("xudp bind fail %d\n", ret);
            return;
        }

        init_ = true;
    }

    void XUdpReceiver::run() {
        for(auto &str : channel_.data_types_) {
            if (StorageMgr::get_instance().get_storager(str) == nullptr) {
                printf("XUdpReceiver::run cannot find data type %s in StorageMgr.\n",
                       str.c_str());
                return;
            }
        }

        if(!init_) {
            printf("XUdpReceiver::run cannot run in case of init failed.\n");
            return;
        }

        int efd = epoll_create(1024);

        epoll_add(x_, efd);

        loop(efd);
        std::cout << "XUdpReceiver::run listen ip:" << channel_.str_ip_
                  << " port:" << channel_.port_ << std::endl;
    }

    void XUdpReceiver::shutdown() {
        xudp_free(x_);
        g_loop.store(false);
    }
} /* namespace common */
} /* namespace forward */