
#include "xudp_sender.h"

namespace forward {
namespace classes {
    XUdpSender::XUdpSender(const nlohmann::json& config)
            : config_(config){   // Must use constructor to create nlohmann::json object.
                                 // Can not use initial list to create nlohmann::json.
                                 // Initial list will add [...] automatically around original object
    }

    void XUdpSender::initialize() {
        for(const auto& item : config_) {
            SendInfo info;
            if(info.initialize(item)) {
                infos_.emplace_back(info);
            }
        }
    }

    void XUdpSender::send(const std::vector<uint8_t>& data) {
        // TODO send data to target
    }
} /* namespace common */
} /* namespace forward */