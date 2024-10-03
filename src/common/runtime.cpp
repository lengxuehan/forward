#include "common/runtime.h"
#include <unistd.h>
#include <csignal>
#include <fstream>
#include "common/exception/errno_exception.h"

namespace forward{
namespace common{
    using namespace forward::common::exception;
    std::mutex Runtime::instance_mutex_{};
    std::unique_ptr<Runtime> Runtime::instance_{nullptr};

    Runtime::Runtime(){
        if (-1 == pipe(selfpipe_)) {
            throw ErrnoException(errno);
        }
    }

    Runtime::~Runtime() noexcept{
        // Stop signal handling
        (void)signal(SIGTERM, SIG_DFL);

        (void)close(selfpipe_[0]);
        (void)close(selfpipe_[1]);
    }

    void Runtime::handle_signal(const int32_t signum) {
        if (SIGTERM == signum) {
            //MEGA_LOG_INFO << "Caught SIGTERM!";
            exit_requested_ = true;
            if (-1 == write(selfpipe_[1], "\0", 1U)) {
                throw ErrnoException(errno);
            }
        }
    }

    void Runtime::signal_handler(const int32_t signum) {
        get_instance().handle_signal(signum);
    }

    void Runtime::wait_until_termination() {
        while (!exit_requested_.load()) {
            int32_t buffer;
            // Blocking read on selfpipe_
            const ssize_t bytes_read(read(selfpipe_[0], &buffer, sizeof(int32_t)));
            if (bytes_read > 0) {
                // selfpipe_ is only used to signal an exit request.
                exit_requested_ = true;
            } else {
                throw ErrnoException(errno);
            }
        }
    }

    Runtime &Runtime::get_instance() {
        return *instance_;
    }

    bool Runtime::load_configuration(const std::string& path_to_config_file, nlohmann::json &config) {
        std::ifstream ifs(path_to_config_file, std::ios::binary);
        if (!ifs.is_open()){
            //MEGA_LOG_ERROR << "failed to open " << path_to_config_file;
            return false;
        }
        config = nlohmann::json::parse(ifs, nullptr, false);
        if (config.is_discarded()){
            //MEGA_LOG_ERROR << "the input is invalid JSON.";
            return false;
        }
        return true;
    }

    bool Runtime::is_initialized() const {
        return initialized_;
    }
}
}