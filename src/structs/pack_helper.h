#pragma once

#include <string>
#include "xudp.h"
#include "nlohmann/json.hpp"

#include "cmd_def.h"

namespace forward{
namespace structs{

class PackHelper {
public:
    static uint32_t parseCmd(const char *p, uint32_t size) {
        if (size < sizeof(Cmd)) {
            return 0;
        }

        Cmd *cmd = (Cmd *)p;
        if (size < cmd->len) {
            return 0;
        }

        return cmd->len;
    }

    static::std::vector<uint8_t> makeupSerializeDataForCmd(const std::string &str, uint16_t no) {
        int len = str.size();
        len += sizeof(Cmd);
        std::vector<uint8_t> data;
        data.resize(len);
        Cmd *c = (Cmd *)data.data();
        c->len = len;
        c->no = no;

        memcpy(c->data, str.c_str(), str.size());

        return std::move(data);
    }
};
}
}