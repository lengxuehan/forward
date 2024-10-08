/** @defgroup RuntimeReceiver
 *  负责付费管理引擎和个性化配置在A核的业务逻辑
 */
/** @addtogroup common
 * \ingroup forward
 *  @{ */
/**
* @file runtime_rvcs.h
* @brief runtime 主程序，负责创建工作线程，控制线程的退出
* @details
* @author		wuting.xu
* @date		    2024/10/03
* @par Copyright(c): 	2024. All rights reserved.
*/

#pragma once

#include <queue>

#include "runtime.h"
#include "classes/xudp_receiver.h"

namespace forward{
namespace common{
class RuntimeReceiver : public Runtime, std::enable_shared_from_this<RuntimeReceiver>{
public:
    ~RuntimeReceiver() override = default;
    /**
     * Delete default copy constructor.
     * Ensure that objects of this type are not copyable and not movable.
     */
    RuntimeReceiver(const RuntimeReceiver&) = delete;
    RuntimeReceiver& operator =(RuntimeReceiver const&) = delete;
    RuntimeReceiver(RuntimeReceiver&&) = delete;
    RuntimeReceiver& operator=(RuntimeReceiver&&) = delete;
    /**
     * \brief Creates an instance of the Runtime.
     *
     * Fills the instance_ pointer and installs a signal handler. This method is thread safe. It is guaranteed that at
     * most one Runtime instance is created, no matter how may times this Function is called in parallel.
     *
     * If instance_ != nullptr, this method does nothing.
     */
    static void make_instance();

    /**
     * \brief Return the Runtime instance.
     */
    static RuntimeReceiver& get_instance();

    /**
     * Read in configuration information and initialize runtime from it.
     */
    void initialize() override;

    /**
     * Destroy the instance of the Runtime.
     */
    void un_initialize() noexcept override;

    /**
     * Start services. After this method has been finished successfully,
     * Create work threads , subscribe someip services and offer someip services provided by SPME master.
     */
    void run() override;

    /**
     * stops service handling, join threads and exit threads. Method returns
     * as soon as services have been stopped.
     */
    void shutdown() override;

    /**
     * \brief 返回RVCS的版本号
     * \return 版本号
     */
    std::string get_forward_version();
protected:
    /**
     * \brief Constructor of RuntimeReceiver.
     *
     * \param config The config object used by RuntimeReceiver.
     */
    explicit RuntimeReceiver(nlohmann::json config);

    /**
     * \brief Parse configuration file master_config.json
     */
    void parse_config();

private:
    nlohmann::json config_;                     // root json object of configuration
    std::string str_forward_version_{};         // forward version
    std::vector<classes::XUdpReceiver> receivers_;

    /**
     * \brief The IPC Communication Manager.
     */
    //std::unique_ptr<ipc_agent::IPCAgent> ipc_agent_{nullptr};
};
}
}
/** @} */ // end of group forward
