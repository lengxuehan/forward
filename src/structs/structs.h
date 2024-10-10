#pragma once

#include <string>

#include "iguana/dynamic.hpp"
#include "tools/packet.h"

namespace forward{
namespace structs{
using namespace forward::tool;
    struct alignas(64) StructA {
        uint64_t ns;        // 本地时间戳,单位纳秒,整数
        double num1, num2; // 随机浮点数
        uint64_t total_id; // 总编号
        uint64_t data_id;  // 子编号

        uint64_t recv_ns;
    };
    YLT_REFL(StructA, ns, num1, num2, total_id, data_id);

    struct alignas(64) StructB {
        uint64_t ns;       // 本地时间戳,单位纳秒,整数
        double num1, num2; //随机浮点数
        char data[64];     // 随机字符数据
        uint64_t total_id; // 总编号
        uint64_t data_id;  // 子编号

        uint64_t recv_ns;
    };
    YLT_REFL(StructB, ns, num1, num2, data[64], total_id, data_id);
}
}