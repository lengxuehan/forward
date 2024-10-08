#pragma once

#include <string>
#include <chrono>
#include <unordered_map>

#include "common/file_utility.h"
#include "data_storager.h"
#include "common/time_sync.h"

using namespace forward::structs;
namespace forward {
namespace classes {
class StorageMgr {
public:
    virtual ~StorageMgr() = default;
    /*
     * Ensure that objects of this type are not copyable and not movable.
    */
    StorageMgr(StorageMgr&& other) = delete;
    StorageMgr(const StorageMgr& other) = delete;
    StorageMgr& operator=(StorageMgr&& other) = delete;
    StorageMgr& operator=(const StorageMgr& other) = delete;

    static StorageMgr& get_instance() {
        static StorageMgr instance;
        return instance;
    }

    void initialize() {
        // 初始化 TimeSync 类
        ts_.init();
        // 启动校准线程
        ts_.start_calibration_thread();
    }

    int64_t get_ns() const {
        return ts_.get_ns();
    }

    void add_storager(const std::string& data_type) {
        std::shared_ptr<DataStorager> ptr{nullptr};
        if(data_type == "StructA") {
            ptr = std::make_shared<StructAStorager>(
                    forward::common::FileUtility::get_process_path(), "csv");
        }
        if(data_type == "StructB") {
            ptr = std::make_shared<StructBStorager>(
                    forward::common::FileUtility::get_process_path(), "csv");
        }

        if(ptr == nullptr) {
            std::cout << "add_storager invalid data_type: " << data_type << std::endl;
            return;
        }
        storagers_[data_type] = ptr;
    }

    void add_storager(const std::string& data_type, std::shared_ptr<DataStorager> storager) {
        storagers_[data_type] = storager;
    }

    std::shared_ptr<DataStorager> get_storager(const char* data_type) {
        return get_storager(std::string(data_type));
    }

    std::shared_ptr<DataStorager> get_storager(const std::string& data_type) {
        if(storagers_.find(data_type) != storagers_.end()) {
            return storagers_[data_type];
        }

        return nullptr;
    }

protected:
    StorageMgr(){

    }
private:
    std::unordered_map<std::string, std::shared_ptr<DataStorager>> storagers_;
    // 初始化纳秒生成器
    forward::common::TimeSync ts_;
};
}
}