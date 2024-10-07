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
#include "structs/sender_channel.h"

namespace forward{
namespace classes{
    class XUdpReceiver {
    public:
        explicit XUdpReceiver(const structs::SenderChannel& channel);
        virtual ~XUdpReceiver() = default;

        /**
         * Read in configuration information and initialize runtime from it.
         */
        void initialize();

        void send(const std::vector<uint8_t>& data) const;

        bool is_ready() const;
    private:
        using SenderChannel = forward::structs::SenderChannel;
        SenderChannel channel_;
        struct addrinfo* to_;
        xudp_channel *ch_{nullptr};
        bool init_{false};
    };
}
}
/** @}*/    // end of group forward