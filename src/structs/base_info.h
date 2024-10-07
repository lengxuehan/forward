/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file base_info.h
* @brief abstract of both send and receiver info
* @details
* @author		wuting.xu
* @date		    2024/10/04
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <string>

#include "xudp.h"
#include "nlohmann/json.hpp"

namespace forward{
namespace structs{

constexpr auto key_channel_id = "channel_id";
constexpr auto key_target_ip = "target_ip";
constexpr auto key_target_port = "target_port";
constexpr auto key_local_ip = "local_ip";
constexpr auto key_local_port = "local_port";
constexpr auto key_data_type = "data_type";

class BaseInfo {
public:
    /**
     * Read in configuration information and initialize runtime from it.
     */
    virtual bool initialize(const nlohmann::json& json_info) = 0;
};
}
}
/** @}*/    // end of group forward