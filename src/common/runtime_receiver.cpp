#include "common/runtime_receiver.h"
#include <csignal>
#include <memory>
#include <string>
#include <utility>
#include <iostream>

#include "common/exception/errno_exception.h"
#include "common/file_utility.h"
#include "structs/receiver_channel.h"
#include "classes/storager_mgr.h"

namespace forward{
namespace common{

    using namespace forward::classes;

    RuntimeReceiver::RuntimeReceiver(nlohmann::json config)
            : config_(std::move(config)){   // Must use constructor to create nlohmann::json object. Can not use initial list to create nlohmann::json. Initial list will add [...] automatically around original object

    }

    void RuntimeReceiver::make_instance(){
        if (instance_ != nullptr){//double checker
            return;
        }
        const std::unique_lock<std::mutex> instance_lock(instance_mutex_);
        if (instance_ != nullptr) {// Another thread was quicker, Instance already exists.
            return;
        }

        // create a new instance
        std::string str_current_dir = FileUtility::get_process_path();
        std::string path_to_config_file{str_current_dir + "/receiver_config.json"};
        nlohmann::json config;     // root of config
        if (!load_configuration(path_to_config_file, config)){
            return;
        }
        instance_ =  std::unique_ptr<RuntimeReceiver>(new RuntimeReceiver(config)); // because creator is protected. Use new to create this object. can't use make_unique

        // Verify whether signal handler was registered successfully. Otherwise, abort
        // the application with an error.
        if (SIG_ERR == signal(SIGTERM, &Runtime::signal_handler)) {
            instance_.reset();
            throw ::forward::common::exception::ErrnoException(errno);
        }
    }

    RuntimeReceiver& RuntimeReceiver::get_instance(){
        if (instance_ == nullptr) {
            RuntimeReceiver::make_instance();
        }
        return *dynamic_cast<RuntimeReceiver*>(instance_.get());
    }

    void RuntimeReceiver::initialize(){
        if (!initialized_) {
            parse_config();

            for(auto& one: receivers_) {
                one.initialize();
            }

            StorageMgr::get_instance().initialize();

            (void)pthread_setname_np(pthread_self(), "forward Main");
            (void)signal(SIGTERM, &signal_handler);
            initialized_ = true;
        }
    }

    void RuntimeReceiver::run(){
        for(auto& one : receivers_) {
            threads_.emplace_back(&XUdpReceiver::run, &one);
        }
    }

    void RuntimeReceiver::shutdown(){
        for(auto& one : receivers_) {
            one.shutdown();
        }
        for (auto& work_thread : threads_) {
            if (work_thread.joinable()) {
                work_thread.join();
            }
        }
    }

    std::string RuntimeReceiver::get_forward_version(){
        return str_forward_version_;
    }

    void RuntimeReceiver::parse_config() {
        if(!config_.contains("receiver_channels")){
            std::cout << "parse_config has no json key: receiver_channels" << std::endl;
            return;
        }

        for(const auto& item : config_["receiver_channels"]) {
            structs::ReceiverChannel info;
            if(info.initialize(item)) {
                for(auto &str : info.data_types_) {
                    StorageMgr::get_instance().add_storager(str);
                }
                receivers_.emplace_back(XUdpReceiver(info));
            }
        }
    }

    void RuntimeReceiver::un_initialize() noexcept {
        if (instance_ != nullptr){
            auto tmp = instance_.release();
            delete tmp;
        }
    }
}
}
