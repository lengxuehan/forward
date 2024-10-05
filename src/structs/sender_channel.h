/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file sender_info.h
* @brief sender info
* @details
* @author		wuting.xu
* @date		    2024/10/04
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <string>

#include "base_info.h"

namespace forward{
namespace structs{
class SenderChannel : public BaseInfo{
public:
    SenderChannel() = default;
    /**
     * Read in configuration information and initialize runtime from it.
     */
    virtual bool initialize(const nlohmann::json& json_info) override;

protected:
    std::string str_ip_{};    // 目标端口
    std::string data_type_{}; // 数据类型
    uint32_t    port_{0};     // 端口
};
}
}
/** @}*/    // end of group forward