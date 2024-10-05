
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
                senders_.emplace_back(sender);
            }
        }
    }

    const std::vector<XUdpSender>& SenderMgr::get_senders() {
        return senders_;
    }
} /* namespace common */
} /* namespace forward */