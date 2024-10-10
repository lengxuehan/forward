#include <iostream>

#include "receiver_channel.h"
#include "tools/json_unity.h"

using JsonUnity = forward::tool::JsonUnity;
namespace forward{
namespace structs{
    bool ReceiverChannel::initialize(const nlohmann::json& json_info) {
        bool res{false};
        res = JsonUnity::get(json_info,key_local_ip, str_ip_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key " << key_local_ip << " failed." << std::endl;
            return false;
        }

        res = JsonUnity::get(json_info,key_local_port, port_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key" << key_local_port << "failed." << std::endl;
            return false;
        }

        res = JsonUnity::get(json_info,key_data_type, data_types_);
        if(!res) {
            std::cout << "ReceiverChannel::initialize get key" << key_data_type << "failed." << std::endl;
            return false;
        }
        return true;
    }
}
}
/** @}*/    // end of group forward