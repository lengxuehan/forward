/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file timer.h
* @brief Implements a Timer that owns a separate thread.
* @details The thread is running as long as the timer is running. However, it
 * consumes CPU time only when the timer
 * fires, is stopped, or its expiration point is updated.
 *
 * Note that it is guaranteed that no locks are held while the stored
 * TimerHandler is called.
* @details
* @author		wuting.xu
* @date		    2024/10/03
* @par Copyright(c): 	2024. All rights reserved.
*/
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <boost/any.hpp>

namespace forward{
namespace common{
class Timer
{
public:
    /**
     * \brief Typedef for the chrono clock used by this Timer.
     */
    typedef std::chrono::steady_clock Clock;

    /**
     * \brief Typedef for the callback called when the timer fires.
     */
    using timer_handler = std::function<void(const boost::any&)>;

    /**
     * \brief Enum to identify the mode of the timer.
     */
    enum Mode: int32_t
    {
        kOneshot,
        kPeriodic,
        kCountPeriodic,
        kBackOff,           //interval is backoffAlgorithm
    };

    /**
     * \brief Constructor to build a new Timer. The new timer is stopped.
     *
     * \param a_timer_handler The callback to call when the timer expires.
     */
    explicit Timer(timer_handler a_timer_handler);

    /**
     * \brief Constructor to build a new Timer. The new timer is stopped.
     *
     * \param a_timer_handler The callback to call when the timer expires.
     */
    Timer(timer_handler a_timer_handler, Mode timer_mode);

    /**
     * \brief Destructor that implicitly stops the timer and joins the timer
     * handler thread.
     */
    virtual ~Timer() noexcept;

    Timer(const Timer&) = delete;
    Timer& operator =(Timer const&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;

    /**
     * \brief Start a timer with the specified delay that stops itself after it
     * fires.
     */
    void start_once(Clock::duration timeout, const boost::any& user_data = boost::any());

    /**
     * \brief Start a periodic timer that fires immediately.
     */
    void start_periodic_immediate(Clock::duration period, const boost::any& user_data = boost::any());

    /**
     * \brief Start a count periodic timer that fires immediately.
     */
    void start_count_periodic_immediate(Clock::duration period, const uint16_t &count, const boost::any& user_data = boost::any());

    /**
     * \brief Start a periodic timer that first fires after one period has passed.
     */
    void start_periodic_delayed(Clock::duration period, const boost::any& user_data = boost::any());

    /**
     * \brief Start a periodic timer that first fires after delay has passed.
     */
    void start_periodic_delayed(Clock::duration period, Clock::duration delay, const boost::any& user_data = boost::any());

    void start_periodic_delayed(Timer::Clock::duration period, uint64_t u_period_count, const boost::any& user_data = boost::any());

    /**
     * \brief Determine whether the timer is currently running, i.e., will fire at
     * some point in the future.
     */
    bool is_running() const;

    /**
     * \brief Determine whether the timer is periodic, i.e., will fire more than
     * once at some point in the future.
     */
    bool is_periodic() const noexcept;

    /**
     * \brief Stop the timer.
     */
    void stop() noexcept;

    /**
     * \brief Stop and join thread run thread of the timer.
     */
    void stop_and_join_run_thread();

    /**
     * \brief Returns the time until the next expiry of the timer. This value is
     * only meaningful if is_running() == true.
     */
    Clock::time_point get_next_expiry_point() const;

    /**
     * \brief Returns the next time the timer will expire. This value is only
     * meaningful if is_running() == true &&
     * is_periodic() == true.
     */
    Clock::duration get_time_remaining() const;
protected:
    /**
     * \brief The Method executed in the thread, firing the actual notification.
     */
    void run();

    /**
     * \brief The Method executed in the thread, firing the actual notification.
     */
    void run_backoff();

    /**
     * \brief The function to call when the timer expires.
     */
    timer_handler timer_handler_;

    /**
     * \brief The mode of the timer
     */
    Mode mode_;

    /**
     * \brief The max count of the timer
     */
    uint16_t max_count_{};

    /**
     * \brief Flag to indicate to thread_ whether it should terminate.
     */
    std::atomic_bool exit_requested_{false};

    /**
     * \brief Flag to indicate whether the timer is currently active.
     */
    std::atomic_bool running_{false};

    /**
     * \brief Flag to indicate whether the work thread joined.
     */
    std::atomic_bool joined_{false};

    /**
     * \brief The period of a periodic timer. The value is only valid if mode ==
     * kPeriodic.
     */
    Clock::duration period_{};

    uint64_t u_start_value_{}, u_period_count_{};

    /**
     * \brief The next point in time when the timer will expire. This value is
     * only valid if running_ == true.
     */
    Clock::time_point next_expiry_point_;

    /**
     * \brief User data. This value is passed to the handler when the timer expires.
     */
    boost::any user_data_;

    /**
     * \brief Mutex used by thread_ to wait on.
     *
     * Declared as mutable so access from const members is thread safe.
     */
    mutable std::mutex mutex_;

    /**
     * \brief Condition Variable used by thread_ to wait on.
     */
    std::condition_variable condition_variable_;

    /**
     * \brief The Thread handling the timer notification.
     */
    std::thread thread_;
};

} /* namespace common */
} /* namespace forward */
/** @}*/    // end of group forward