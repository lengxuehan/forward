#include <netdb.h>
#include <iostream>
#include "xudp.h"
#include "xudp_sender.h"

namespace forward {
namespace classes {
    XUdpSender::XUdpSender(const structs::SenderChannel& channel)
            : channel_(channel){
    }

    void XUdpSender::initialize() {
        std::cout << "XUdpSender::initialize target ip:" << channel_.str_ip_
            << " port:" << channel_.port_ << std::endl;
        int ret = getaddrinfo(channel_.str_ip_.c_str(),
                              std::to_string(channel_.port_).c_str(), NULL, &to_);
        if (ret) {
            printf("getaddrinfo err ip:%s port:%d\n", channel_.str_ip_.c_str(), channel_.port_);
            return;
        }

        init_ = true;
    }

    void XUdpSender::set_channel(xudp_channel *ch) {
        ch_ = ch;
    }

    void XUdpSender::send(const std::vector<uint8_t>& data) const {
        if(!init_) {
            printf("XUdpSender init failed. ip:%s port:%d\n",
                   channel_.str_ip_.c_str(), channel_.port_);
            return;
        }
        if(!ch_) {
            printf("XUdpSender xudp_channel nullptr. channel id:%d\n", channel_.channel_id_);
            return;
        }
        int ret = xudp_send_channel(ch_, (char*)data.data(), data.size(), to_->ai_addr, 0);
        if (ret < 0) {
            printf("xudp_send_one fail. %d\n", ret);
        }
        xudp_commit_channel(ch_);
    }

    bool XUdpSender::is_ready() const {
        if(!ch_ || !init_) {
            return false;
        }
        return true;
    }
} /* namespace common */
} /* namespace forward */