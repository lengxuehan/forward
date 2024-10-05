#pragma once

#include <string>

namespace forward{
namespace structs{
struct alignas(64) StructB {
    // 成员变量
    uint64_t ns;       // 本地时间戳,单位纳秒,整数
    double num1, num2; //随机浮点数
    char data[64];     // 随机字符数据
    uint64_t total_id; // 总编号
    uint64_t data_id;  // 子编号

    void serialize(std::vector<uint8_t>& vec_data) {
        vec_data.clear();
        Packet packet(vec_data);
        packet << ns << num1 << num2;
        packet.append((uint8_t*)data, 64);
        packet << total_id << data_id;
    }
};
}
}