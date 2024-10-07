/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file json unity.h
* @brief json unity tool
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
namespace tool{

class JsonUnity {
public:
    JsonUnity(const JsonUnity&) = delete;
    JsonUnity& operator =(JsonUnity const&) = delete;
    JsonUnity(JsonUnity&&) = delete;
    JsonUnity& operator=(JsonUnity&&) = delete;
    /**
     * @param true successfully get out value.
     * @param false get out value failed, in case of wrong json format
     */
    static bool get(const nlohmann::json& json, const std::string& key,std::string& str_out) {
        if(json.contains(key) && json[key].is_string()) {
            str_out = json.at(key);
            return true;
        }
        return false;
    }

    static bool get(const nlohmann::json& json, const std::string& key, uint32_t& out) {
        if(json.contains(key) && json[key].is_number_integer()) {
            out = json[key];
            return true;
        }
        return false;
    }

protected:
    std::string str_ip_{};
    std::string data_type_{};
    uint32_t    port_{0};
};
}
}
/** @}*/    // end of group forward