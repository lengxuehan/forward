/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file xudp_sender.h
* @brief utility of file operation for forward
* @details
* @author		wuting.xu
* @date		    2024/10/04
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <string>

#include "xudp.h"
#include "nlohmann/json.hpp"
#include "structs/send_info.h"

namespace forward{
namespace classes{
class XUdpSender {
public:
    explicit XUdpSender(const nlohmann::json& config);
    virtual ~XUdpSender() = default;
    XUdpSender(XUdpSender const&) = delete;
    XUdpSender& operator =(XUdpSender const&) = delete;
    XUdpSender(XUdpSender&&) = delete;
    XUdpSender& operator=(XUdpSender&&) = delete;

    /**
     * Read in configuration information and initialize runtime from it.
     */
    void initialize();

    void send(const std::vector<uint8_t>& data);
private:
    const nlohmann::json& config_;                     // sender json object of configuration
    using SendInfo = forward::structs::SendInfo;
    std::vector<SendInfo> infos_;
};
}
}
/** @}*/    // end of group forward