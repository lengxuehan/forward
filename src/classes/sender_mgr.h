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
#include "xudp_sender.h"

namespace forward{
namespace classes{
class SenderMgr {
public:
    explicit SenderMgr(const nlohmann::json& config);
    virtual ~SenderMgr() = default;

    SenderMgr(SenderMgr const&) = delete;
    SenderMgr& operator =(SenderMgr const&) = delete;
    SenderMgr(SenderMgr&&) = delete;
    SenderMgr& operator=(SenderMgr&&) = delete;
    /**
     * Read in configuration information and initialize runtime from it.
     */
    void initialize();

    const std::vector<XUdpSender>& get_senders();
protected:

private:
    const nlohmann::json& config_;        // sender json object of configuration
    std::vector<XUdpSender> senders_;
};
}
}
/** @}*/    // end of group forward