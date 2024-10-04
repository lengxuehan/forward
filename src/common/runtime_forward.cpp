#include "common/runtime_forward.h"
#include <csignal>
#include <memory>
#include <string>
#include <utility>
#include "common/exception/errno_exception.h"
#include "common/file_utility.h"
#include "common/time_sync.h"

namespace forward{
namespace common{

    RuntimeForward::RuntimeForward(nlohmann::json config)
            : config_(std::move(config)){   // Must use constructor to create nlohmann::json object. Can not use initial list to create nlohmann::json. Initial list will add [...] automatically around original object

    }

    void RuntimeForward::make_instance(){
        if (instance_ != nullptr){//double checker
            return;
        }
        const std::unique_lock<std::mutex> instance_lock(instance_mutex_);
        if (instance_ != nullptr) {// Another thread was quicker, Instance already exists.
            return;
        }

        // create a new instance
        std::string str_current_dir = FileUtility::get_process_path();
        std::string path_to_config_file{str_current_dir + "/forward_config.json"};
        nlohmann::json config;     // root of config
        if (!load_configuration(path_to_config_file, config)){
            return;
        }
        instance_ =  std::unique_ptr<RuntimeForward>(new RuntimeForward(config)); // because creator is protected. Use new to create this object. can't use make_unique

        // Verify whether signal handler was registered successfully. Otherwise, abort
        // the application with an error.
        if (SIG_ERR == signal(SIGTERM, &Runtime::signal_handler)) {
            instance_.reset();
            throw ::forward::common::exception::ErrnoException(errno);
        }
    }

    RuntimeForward& RuntimeForward::get_instance(){
        if (instance_ == nullptr) {
            RuntimeForward::make_instance();
        }
        return *dynamic_cast<RuntimeForward*>(instance_.get());
    }

    void RuntimeForward::initialize(){
        if (!initialized_) {
            parse_config();

            (void)pthread_setname_np(pthread_self(), "forward Main");
            (void)signal(SIGTERM, &signal_handler);
            initialized_ = true;
        }
    }

    void RuntimeForward::run(){
        // TODO
    }

    void RuntimeForward::shutdown(){
        for (auto& work_thread : threads_) {
            if (work_thread.joinable()) {
                work_thread.join();
            }
        }
    }

    std::string RuntimeForward::get_forward_version(){
        return str_forward_version_;
    }

    void RuntimeForward::parse_config() {

    }

    void RuntimeForward::un_initialize() noexcept {
        if (instance_ != nullptr){
            auto tmp = instance_.release();
            delete tmp;
        }
    }
}
}
