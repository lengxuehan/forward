#include <iostream>

#include "receiver_channel.h"
#include "tools/json_unity.h"

using JsonUnity = forward::tool::JsonUnity;
namespace forward{
namespace structs{
    bool ReceiverChannel::initialize(const nlohmann::json& json_info) {
        bool res{false};
        res = JsonUnity::get(json_info,key_target_ip, str_ip_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key " << key_target_ip << " failed." << std::endl;
            return false;
        }

        res = JsonUnity::get(json_info,key_target_port, port_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key" << key_target_port << "failed." << std::endl;
            return false;
        }

        res = JsonUnity::get(json_info,key_target_port, channel_id_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key" << key_target_port << "failed." << std::endl;
            return false;
        }
        return true;
    }
}
}
/** @}*/    // end of group forward