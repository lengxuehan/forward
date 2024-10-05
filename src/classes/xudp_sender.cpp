
#include "xudp_sender.h"

namespace forward {
namespace classes {
    XUdpSender::XUdpSender(const structs::SenderChannel& channel)
            : channel_(channel){
    }

    void XUdpSender::send(const std::vector<uint8_t>& data) const{
        // TODO send data to target
    }
} /* namespace common */
} /* namespace forward */