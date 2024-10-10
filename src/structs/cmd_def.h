#pragma once

#include <string>

#include "xudp.h"
#include "nlohmann/json.hpp"

#include "structs.h"
#include "classes/storager_mgr.h"

namespace forward{
namespace structs{
    using namespace forward::classes;
#pragma pack(1)
    struct Cmd {
        uint16_t no;
        uint16_t len;
        char	data[0];
    };
#pragma pack()

    template <uint8_t no>
    struct UniqueTrailer {};

#define CMD_DECLARE(cmd_name, data_type, n)	\
	template <> struct UniqueTrailer<n> {};	\
	struct cmd_name {	\
		const static uint16_t no = n;	\
		cmd_name() {	\
		}	\
		data_type	data;               \
	};

    CMD_DECLARE(StructACmd, StructA, 1);
    CMD_DECLARE(StructBCmd, StructB, 2);
}
}