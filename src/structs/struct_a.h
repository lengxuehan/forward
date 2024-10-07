#pragma once

#include <string>

#include "tools/packet.h"


namespace forward{
namespace structs{
using namespace forward::tool;
struct alignas(64) StructA {
    // 成员变量
    uint64_t ns;        // 本地时间戳,单位纳秒,整数
    double num1, num2; // 随机浮点数
    uint64_t total_id; // 总编号
    uint64_t data_id;  // 子编号

    void serialize(std::vector<uint8_t>& data) {
        data.clear();
        Packet packet(data);
        packet << ns << num1 << num2 << total_id << data_id;
    }

    void deserialize(const char* p, uint32_t size) {
        Packet packet((uint8_t*)p, size);
        packet >> ns >> num1 >> num2 >> total_id >> data_id;
    }

    uint64_t recv_ns;
};
}
}