/** @addtogroup common
 * \ingroup forward
 *  @{
 */
/**
* @file timer.h
* @brief Implements a TimeSync that get calibrated time.
*
* Note that it is guaranteed that no locks are held while the stored
* TimerHandler is called.
* @details
* @author		wuting.xu
* @date		    2024/10/04
* @par Copyright(c): 	2024. All rights reserved.
*/
#pragma once

#include <chrono>
#include <atomic>
#include <thread>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace forward{
namespace common{
class TimeSync {
public:
    static const int64_t NsPerSec = 1000000000; // 每秒纳秒数

    // 初始化函数，默认校验时间为1秒
    void init(int64_t init_calibrate_ns = 1 * NsPerSec, int64_t calibrate_interval_ns = 3 * NsPerSec) {
        calibrate_interval_ns_ = calibrate_interval_ns;
        int64_t base_tsc, base_ns;
        syncTime(base_tsc, base_ns);
        int64_t expire_ns = base_ns + init_calibrate_ns;
        while (get_sys_ns() < expire_ns) std::this_thread::yield();
        int64_t delayed_tsc, delayed_ns;
        syncTime(delayed_tsc, delayed_ns);
        double init_ns_per_tsc = (double) (delayed_ns - base_ns) / (delayed_tsc - base_tsc);
        saveParam(base_tsc, base_ns, base_ns, init_ns_per_tsc);
    }

    // 校准函数
    void calibrate() {
        if (rdtsc() < next_calibrate_tsc_) return;
        int64_t tsc, ns;
        syncTime(tsc, ns);
        int64_t calculated_ns = tsc2ns(tsc);
        int64_t ns_err = calculated_ns - ns;
        int64_t expected_err_at_next_calibration =
                ns_err + (ns_err - base_ns_err_) * calibrate_interval_ns_ / (ns - base_ns_ + base_ns_err_);
        double new_ns_per_tsc =
                ns_per_tsc_ * (1.0 - (double) expected_err_at_next_calibration / calibrate_interval_ns_);
        saveParam(tsc, calculated_ns, ns, new_ns_per_tsc);
    }

    // 读取时间戳计数器
    static inline int64_t rdtsc() {
#ifdef _MSC_VER
        return __rdtsc();
#elif defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
        return __builtin_ia32_rdtsc();
#else
        return rdsysns();
#endif
    }

    // 将TSC值转换为纳秒
    inline int64_t tsc2ns(int64_t tsc) const {
        while (true) {
            uint32_t before_seq = param_seq_.load(std::memory_order_acquire) & ~1;
            std::atomic_signal_fence(std::memory_order_acq_rel);
            int64_t ns = base_ns_ + (int64_t) ((tsc - base_tsc_) * ns_per_tsc_);
            std::atomic_signal_fence(std::memory_order_acq_rel);
            uint32_t after_seq = param_seq_.load(std::memory_order_acquire);
            if (before_seq == after_seq) return ns;
        }
    }

    // 获取当前纳秒时间戳
    inline int64_t get_ns() const { return tsc2ns(rdtsc()); }

    // 读取系统纳秒时间戳
    static inline int64_t get_sys_ns() {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    }

    // 获取TSC的GHz频率
    double getTscGhz() const { return 1.0 / ns_per_tsc_; }

    // 同步时间函数
    static void syncTime(int64_t &tsc_out, int64_t &ns_out) {
#ifdef _MSC_VER
        const int N = 15;
#else
        const int N = 3;
#endif
        int64_t tsc[N + 1];
        int64_t ns[N + 1];

        tsc[0] = rdtsc();
        for (int i = 1; i <= N; i++) {
            ns[i] = get_sys_ns();
            tsc[i] = rdtsc();
        }

#ifdef _MSC_VER
        int j = 1;
        for (int i = 2; i <= N; i++) {
          if (ns[i] == ns[i - 1]) continue;
          tsc[j - 1] = tsc[i - 1];
          ns[j++] = ns[i];
        }
        j--;
#else
        int j = N + 1;
#endif

        int best = 1;
        for (int i = 2; i < j; i++) {
            if (tsc[i] - tsc[i - 1] < tsc[best] - tsc[best - 1]) best = i;
        }
        tsc_out = (tsc[best] + tsc[best - 1]) >> 1;
        ns_out = ns[best];
    }

    // 保存参数函数
    void saveParam(int64_t base_tsc, int64_t base_ns, int64_t sys_ns, double new_ns_per_tsc) {
        base_ns_err_ = base_ns - sys_ns;
        next_calibrate_tsc_ = base_tsc + (int64_t) ((calibrate_interval_ns_ - 1000) / new_ns_per_tsc);
        uint32_t seq = param_seq_.load(std::memory_order_relaxed);
        param_seq_.store(++seq, std::memory_order_release);
        std::atomic_signal_fence(std::memory_order_acq_rel);
        base_tsc_ = base_tsc;
        base_ns_ = base_ns;
        ns_per_tsc_ = new_ns_per_tsc;
        std::atomic_signal_fence(std::memory_order_acq_rel);
        param_seq_.store(++seq, std::memory_order_release);
    }

    std::atomic<bool> running{false};
    std::thread calibration_thread;

    // 启动校准线程
    void start_calibration_thread() {
        running = true;
        calibration_thread = std::thread([this]() {
            while (running) {
                this->calibrate();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    // 停止校准线程
    void stop_calibration_thread() {
        running = false;
        if (calibration_thread.joinable()) {
            calibration_thread.join();
        }
    }

    // 析构函数，确保线程停止
    ~TimeSync() {
        stop_calibration_thread();
    }

    alignas(64) std::atomic<uint32_t> param_seq_{0};  // 参数序列号
    double ns_per_tsc_;                               // 每个TSC的纳秒数
    int64_t base_tsc_;                                // 基准TSC值
    int64_t base_ns_;                                 // 基准纳秒值
    int64_t calibrate_interval_ns_;                    // 校准间隔（纳秒）
    int64_t base_ns_err_;                             // 基准纳秒误差
    int64_t next_calibrate_tsc_;                      // 下次校准的TSC值
};
}
}
/** @}*/    // end of group forward