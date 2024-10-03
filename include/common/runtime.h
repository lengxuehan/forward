/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file runtime.h
* @brief class Runtime is a singleton, which represents the running instance of the runtime master module.
* @details It basically provides methods for controlling the life cycle of the runtime master
*  - that is: initializing, starting/running and stopping.
* @author		wuting.xu
* @date		    2024/10/03
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include "nlohmann/json.hpp"

namespace forward{
namespace common{
class Runtime
{
public:
    /**
     * \brief Class Runtime constructor.
     */
    Runtime();
    virtual ~Runtime() noexcept;
    /*
     * Ensure that objects of this type are not copyable and not movable.
     */
    Runtime(Runtime&& other) = delete;
    Runtime(const Runtime& other) = delete;
    Runtime& operator=(Runtime&& other) = delete;
    Runtime& operator=(const Runtime& other) = delete;

    /**
     * \brief Performs one-time setup of the application.
     */
    virtual void initialize() = 0;
    /**
     * \brief Performs one-time setdown of the application.
     */
    virtual void un_initialize() = 0;
    /**
     * \brief Executes the workload of the application.
     *
     * This method spawns threads for CommandHandler::main.
     */
    virtual void run() = 0;

    /**
     * \brief Perform an orderly shutdown of the application.
     *
     * This method shuts down all communication and waits for all threads to terminate.
     */
    virtual void shutdown() = 0;

    /**
     * \brief Return the Runtime instance.
     */
    static Runtime& get_instance();

    /**
     * \brief Getter for the initialization status.
     */
    bool is_initialized() const;

    /**
     * \brief Blocks the application until it is explicitly asked to terminate.
     *
     * The blocking wait is implemented by a blocking wait on a pipe. HandleSignal() catches the SIGTERM signal and
     * converts it into a token sent via the pipe.
     */
    void wait_until_termination();

    /**
     * \brief Asks the application to load the configuration from the configuration file.
     *
     * \param path_to_config_file path to the file location of the config file.
     *
     * @throws ParseError In case the config file could not be opened or the syntax of the config file is invalid.
     */
    static bool load_configuration(const std::string& path_to_config_file, nlohmann::json &config);

    /**
     * \brief Signal handler to catch signals sent to the process. In order to give the invoker an opportunity to actively exit the program
     *
     * This handler delegates to HandleSignal(int32_t).
     */
    static void signal_handler(int32_t signum);

    /**
     * \brief Signal handler to catch signals sent to the AdaptiveAutosarApplication.
     *
     * This signal handler only handles SIGTERM, which asks the application to terminate.
     * It forwards the notification that a SIGTERM occurred using selfpipe.
     */
    void handle_signal(int32_t signum);
protected:
    /**
     * \brief Container to hold all Threads spawned by this object.
     */
    std::vector<std::thread> threads_{};

    /**
     * \brief Mutex used to protect concurrent write access to instance_.
     *
     * Note that concurrent read is possible without protection by the mutex. The rationale is that instance_ is
     * conceptually write-once, i.e., it will be written once by MakeInstance() and is read-only from then on out.
     */
    static std::mutex instance_mutex_;

    /**
     * \brief A pair of pipes used by SignalHandler() to notify WaitSignal() of an incoming signal.
     */
    int32_t selfpipe_[2]{};

    /**
     * \brief Pointer to the instance of the AdaptiveAutosarApplication.
     *
     * Used by SignalHandler(int32_t) to delegate to HandleSignal(int32_t). Note: instance_ is a guard against misconfigured
     * the signal handler. It is not an implementation of the singleton pattern!
     */
    static std::unique_ptr<Runtime> instance_;

    /**
     * \brief Flag to indicate that the Runtime object is initialized correctly.
     */
    bool initialized_{false};

    /**
     * \brief Flag to indicate whether this Application has been asked to terminate.
     */
    std::atomic_bool exit_requested_{false};
};
} /* namespace common */
} /* namespace forward */
/** @}*/    // end of group forward