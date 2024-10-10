#include <iostream>
#include <vector>
#include <fstream>

#include "structs/pack_helper.h"
#include "structs/sender_channel.h"

#include "common/time_sync.h"
#include "xudp_sender.h"
#include "common/file_utility.h"
#include "sender_mgr.h"
#include "iguana/iguana.hpp"
#include "structs/cmd_def.h"

using namespace forward::classes;
using namespace forward::structs;

int main() {
    // 1.解析配置文件到结构体中
    std::string str_current_dir = forward::common::FileUtility::get_process_path();
    std::string path_to_config_file{str_current_dir + "/sender_config.json"};
    std::ifstream ifs(path_to_config_file, std::ios::binary);
    if (!ifs.is_open()){
        std::cout << "failed to open " << path_to_config_file << std::endl;
        return 0;
    }
    nlohmann::json config = nlohmann::json::parse(ifs, nullptr, false);
    if (config.is_discarded()){
        std::cout << "the input is invalid JSON.";
        return 0;
    }

    SenderMgr sender_mgr(config["sender_channels"]);
    sender_mgr.initialize();
    sender_mgr.set_channel();
    
    // 2.初始化发送器
    const std::vector<XUdpSender>&  senders = sender_mgr.get_senders();
    
    // 3.初始化纳秒生成器
    forward::common::TimeSync ts;
    // 初始化 TimeSync 类
    ts.init();
    // 启动校准线程
    ts.start_calibration_thread();

    // 4.循环发送数据
    int64_t total_id = 1;   // 总编号
    int64_t data_a_id = 1;  // 子编号
    int64_t before_ns = 0;  // 事前纳秒
    int64_t using_ns = 0;   // 用时纳秒
    int64_t total_ns = 0;   // 总用时纳秒
    int64_t average_ns;     // 平均发送用时纳秒

    srand(time(NULL));

    if(senders.size() < 1) {
        std::cout << "senders size is: " << senders.size() << std::endl;
        return 0;
    }

    while (true) {
        StructACmd data_a;
        data_a.data.ns = ts.get_ns(); // 本地时间戳,单位纳秒,整数
        data_a.data.num1 =  rand() / 10000.0; // 随机浮点数
        data_a.data.num2 =  rand() / 10000.0; // 随机浮点数
        data_a.data.total_id = total_id; // 总编号
        data_a.data.data_id = data_a_id; // 子编号

        // 序列化数据
        std::string s;
        iguana::to_pb(data_a.data, s);
        std::vector<uint8_t> data = PackHelper::makeupSerializeDataForCmd(s, data_a.no);

        //printf("total id : %lu\n", data_a.ns);
        //printf("total id : %lu\n", data_a.total_id);
        //printf("total id : %lu\n", data_a.data_id);

        // 调用指定编号发送数据
        before_ns = ts.get_ns();
        if(senders[0].is_ready()) {
            senders[0].send(data);
            using_ns = ts.get_ns() - before_ns;
            total_ns += using_ns;
        }

        if(senders.size() > 1) {
            data_a_id++;
            StructBCmd data_b;
            data_b.data.ns = ts.get_ns(); // 本地时间戳,单位纳秒,整数
            data_b.data.num1 =  rand() / 10000.0; // 随机浮点数
            data_b.data.num2 =  rand() / 10000.0; // 随机浮点数
            std::string hello{"hello"};
            hello += std::to_string(data_a_id);
            int idx = 0;
            for(auto c : hello) {
                data_b.data.data[idx++] = c;
            }
            data_b.data.total_id = total_id; // 总编号
            data_b.data.data_id = data_a_id; // 子编号

            // 序列化数据
            iguana::to_pb(data_b.data, s);
            data = PackHelper::makeupSerializeDataForCmd(s, data_b.no);

            before_ns = ts.get_ns();
            if(senders[1].is_ready()) {
                senders[0].send(data);
                using_ns = ts.get_ns() - before_ns;
                total_ns += using_ns;
            }
        }

        data_a_id++;
        total_id++;

        // 计算平均发送用时
        int64_t average_times = 1000000;
        if (total_id % average_times == 0) {
            average_ns = total_ns / average_times;
            std::cout << "average_ns send time: " << average_ns << " ns" << std::endl;

            total_ns = 0; // 重置总用时
        }
    }
    return 0;
}
