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

#include "nlohmann/json.hpp"
#include "xudp.h"
#include "structs/receiver_channel.h"

namespace forward{
namespace classes{
    class XUdpReceiver {
    public:
        explicit XUdpReceiver(const structs::ReceiverChannel& channel);
        virtual ~XUdpReceiver() = default;

        /**
         * Read in configuration information and initialize runtime from it.
         */
        void initialize();

        void run();

        void shutdown();

    private:
        using ReceiverChannel = forward::structs::ReceiverChannel;
        ReceiverChannel channel_;
        struct addrinfo* addr_info_;
        bool init_{false};
        xudp *x_{nullptr};
    };
}
}
/** @}*/    // end of group forward