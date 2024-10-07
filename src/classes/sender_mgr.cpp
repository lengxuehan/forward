
#include <iostream>
#include <netdb.h>
#include "sender_mgr.h"

namespace forward {
namespace classes {
    SenderMgr::SenderMgr(const nlohmann::json& config)
            : config_(config){   // Must use constructor to create nlohmann::json object.
                                 // Can not use initial list to create nlohmann::json.
                                 // Initial list will add [...] automatically around original object
    }

    void SenderMgr::initialize() {
        for(const auto& item : config_) {
            structs::SenderChannel info;
            if(info.initialize(item)) {
                XUdpSender sender(info);
                sender.initialize();
                senders_.emplace_back(sender);
            }
        }
    }

    void SenderMgr::set_channel() {
        xudp *x;
        xudp_conf conf = {};
        conf.group_num     = 1;
        conf.log_with_time = true;
        conf.log_level = XUDP_LOG_WARN;
        x = xudp_init(&conf, sizeof(conf));
        if(x == nullptr) {
            std::cout << "XUdpSender::initialize xudp_init failed." << std::endl;
            return;
        }

        struct addrinfo* tmp;
        int ret = getaddrinfo("172.18.0.212", "0", NULL, &tmp);
        if (ret) {
            printf("XUdpSender::initialize getaddrinfo err ip: 0.0.0.0\n");
            return;
        }
        int size;
        if (tmp->ai_family == AF_INET) {
            printf("AF_INET addr.\n");
            size = sizeof(struct sockaddr_in);
        } else {
            printf("AF_INET6 addr.\n");
            size = sizeof(struct sockaddr_in6);
        }
        ret = xudp_bind(x, (struct sockaddr *)tmp->ai_addr, size, 1);
        if (ret) {
            xudp_free(x);
            printf("xudp bind fail %d\n", ret);
            return;
        }

        xudp_group *g;
        g = xudp_group_get(x, 0);
        if(g == nullptr) {
            std::cout << "XUdpSender::initialize xudp_group_get failed." << std::endl;
            return;
        }
        xudp_channel *ch = xudp_group_channel_first(g);
        for(auto& sender : senders_) {
            sender.set_channel(ch);
        }
    }

    const std::vector<XUdpSender>& SenderMgr::get_senders() {
        return senders_;
    }
} /* namespace common */
} /* namespace forward */