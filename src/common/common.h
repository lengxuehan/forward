/**
* @file common.h
* @brief common defines.
* @author       wuting.xu
* @date         2023/10/30
* @par Copyright(c):    2023 megatronix. All rights reserved.
*/

#pragma once

#include <vector>
#include <string>
#include <array>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <string.h>

namespace forward{
namespace tool{
    enum class wakeup_type_t {
        fired,    // 点火
        calling,  // 来电或者短信唤醒
    };

#undef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
TypeName(const TypeName&);                           \
void operator=(const TypeName&)

    // A macro to disallow all the implicit constructors, namely the
    // default constructor, copy constructor and operator= functions.
    //
    // This should be used in the private: declarations for a class
    // that wants to prevent anyone from instantiating it. This is
    // especially useful for classes containing only static methods.
#undef DISALLOW_IMPLICIT_CONSTRUCTORS
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName)    \
TypeName();                                     \
DISALLOW_EVIL_CONSTRUCTORS(TypeName)

    /**
     * @return 以秒为单位
     * */
    uint64_t get_timestamp();
    /**
     * 判断是否为同一天
     * @param old_tm 上一次时间, 以秒为单位
     * @param new_tm 现在时间, 以秒为单位
     * @return true 同一天; false 不是同一天
     */
    bool is_same_day(const uint64_t old_tm, const uint64_t new_tm);

    class EndianChecker
    {
    public:
        static bool isLittleEndianHost()
        {
            std::uint32_t test_number = 0x1;
            auto* test_bytes = reinterpret_cast<std::uint8_t*>(&test_number);
            return (test_bytes[0] == 1);
        }
    };
} // end of namespace cloud_manager
}