/*
做一个C++的类，可以异步写入 结构体到 H5或scv文件
1.初始化这个类接收3个参数，一个是结构体的类型data_type，一个是文件夹的路径dir，一个是文件的类型file_type（H5或scv）
2.这个类有一个异步写入的函数，接收一个结构体的指针，然后把这个结构体写入到文件中。只会有1个线程调用写入。
3.结构体总共有两种类型，一种是depth5,一种是trades，格式如下：
enum class Exchange : int8_t {
    BINANCE_FUTURE,
    OKX_FUTURE,
    BITGET_FUTURE,
    // ... 其他可能的交易所
};

enum class Direction : int8_t {
    BUY,
    SELL
};

// 优化后的 Depth5 结构
struct alignas(64) Depth5 {
    std::array<std::pair<double, double>, 5> bids;  // 买单
    std::array<std::pair<double, double>, 5> asks;  // 卖单
    int64_t ets;      // 交易所时间戳,单位毫秒,整数
    int64_t lts;      // 本地时间戳,单位毫秒,整数
    char symbol[16];  // 交易对名称 BTC-USDT ETH-USDT
    Exchange exchange;  // 交易所市场名称,枚举类型
};

// 优化后的 TradesData 结构
struct alignas(64) TradesData {
    int64_t ets;      // 交易所时间戳,单位毫秒,整数
    int64_t lts;      // 本地时间戳,单位毫秒,整数
    double price;    // 价格
    double volume;   // 数量
    char symbol[16];  // 交易对名称
    Exchange exchange;  // 交易所市场名称
    Direction direction;  // 交易方向
};

4.写入路径格式为：路径dir/交易所名称/结构体的类型/交易对名称/日期.文件类型
例如 /data_dir/binance-f/depth5/BTC-USDT/2021-01-01.h5
5.也就是说，每个交易所，每个结构体类型，每个交易对，每天一个文件，
其中日期使用交易所时间 ets 的日期，如果ets是2021-01-01 00:00:00，那么就是2021-01-01

6.写入的时候，如果文件不存在，就创建文件，如果文件存在，就追加写入
数据不用立刻写入，而是缓存足够的条数(例如类定义一个常量 100条) 后再写入。
*/

#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <map>
#include <mutex>
#include <boost/any.hpp>

#include "structs/struct_a.h"
#include "structs/struct_b.h"

using namespace forward::structs;

class DataStorager {
public:
    DataStorager(const std::string& dir, const std::string& file_type)
        : dir_(dir), file_type_(file_type) {}

    virtual void asyncWrite(const boost::any& data) = 0;
protected:
    virtual void flushBuffer() = 0;

    std::string getDateFromTimestamp(int64_t ts) {
        std::time_t time = static_cast<std::time_t>(ts / 1000);
        std::tm* tm = std::localtime(&time);
        char date[11];
        std::strftime(date, sizeof(date), "%Y-%m-%d", tm);
        return std::string(date);
    }

    std::filesystem::path generatePath(const std::string& date) {
        std::filesystem::path p = dir_;
        p /= data_type_;
        p += "/" + date + "." + file_type_;
        return p;
    }

    void closeFile(const std::string& path, const std::string& file_type) {
        if (file_type == "csv") {
            if (csv_open_files_.find(path) != csv_open_files_.end()) {
                csv_open_files_[path].close();
                csv_open_files_.erase(path);
            }
        } else if (file_type == "h5" || file_type == "hdf5") {
            // 如果未来有其他文件类型的处理逻辑，请在这里添加
        }
    }

    std::string dir_;
    std::string file_type_;
    std::string data_type_;
    std::map<std::string, std::ofstream> csv_open_files_;

    static constexpr size_t MAX_BUFFER_SIZE = 600;
    std::string last_date_;
    std::mutex mutex_;
};

class StructAStorager : public DataStorager {
public:
    StructAStorager(const std::string& dir, const std::string& file_type)
        : DataStorager(dir, file_type) {
            data_type_ = "StructA";
        }

    ~StructAStorager() {
        std::lock_guard<std::mutex> lock(mutex_);  // 确保在此过程中没有其他线程尝试写入

        flushBuffer();  // 保存缓冲区数据

        for (auto& [path, file] : csv_open_files_) {
            if (file.is_open()) {
                file.close();  // 关闭文件
            }
        }
        csv_open_files_.clear();  // 清空map
    }
    void asyncWrite(const boost::any& data) override {
        try {
            StructA data_a = boost::any_cast<StructA>(data);
            asyncWrite(data_a);
        } catch (const boost::bad_any_cast& e) {
            std::cout << "Type mismatch: " << e.what() << std::endl;
        }

    }
protected:
    std::vector<StructA> strucA_buffer_;

    void asyncWrite(const StructA& data) {
        std::string date = getDateFromTimestamp(data.ns);

        std::lock_guard<std::mutex> lock(mutex_); // 使用互斥锁确保线程安全

        // 在将新数据添加到缓冲区之前，首先检查日期是否发生了变化
        if (last_date_ != "" && last_date_ != date) {
            flushBuffer();  // 如果日期发生了变化，先保存缓冲区的数据
            last_date_ = date;  // 更新日期记录
        }

        // 将新数据添加到缓冲区
        strucA_buffer_.push_back(data);

        // 检查缓冲区是否已达到最大大小
        if (strucA_buffer_.size() == MAX_BUFFER_SIZE) {
            flushBuffer();  // 当缓冲区满时，保存缓冲区的数据
        }
    }

    std::string formatFloat(double value) {
        std::stringstream ss;
        ss << std::fixed << value;
        std::string s = ss.str();
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }

    void writeToCSV(const std::filesystem::path& p, const std::vector<StructA>& data) {
        if (csv_open_files_.find(p.string()) == csv_open_files_.end()) {
            std::filesystem::create_directories(p.parent_path());
            csv_open_files_[p.string()] = std::ofstream(p, std::ios::app);
        }
        std::ofstream& file = csv_open_files_[p.string()];

        std::stringstream buffer;  // 使用一个字符串流来缓冲数据

        for (const auto& entry : data) {
            buffer << entry.ns << "," << entry.recv_ns << ","
                << entry.num1 << "," << entry.num2 << entry.num1 << ","
                << entry.total_id << "," << entry.data_id;

            // 移除最后一个逗号
            //buffer.seekp(-1, std::ios_base::cur);
            buffer << "\n";
        }

        file << buffer.str();  // 一次性将所有数据写入文件
        file.flush();          // 确保数据已经写入文件
    }

    void flushBuffer() override {
        // 确定路径和文件名
        StructA& first_data = strucA_buffer_[0];
        std::string cur_data = getDateFromTimestamp(first_data.ns);
        std::filesystem::path p = generatePath(cur_data);  // 使用一个函数来生成路径
        printf("generatePath %s\n", p.string().c_str());

        // 如果日期变化，关闭旧的文件句柄
        if (last_date_!= "" && cur_data != last_date_) {
            std::filesystem::path last_path = generatePath(last_date_);
            closeFile(last_path.string(), file_type_);
            last_date_ = cur_data;
        }

        // 根据file_type_决定如何写入
        if (file_type_ == "csv") {
            writeToCSV(p, strucA_buffer_);
        } else if (file_type_ == "h5" || file_type_ == "hdf5") {
            // hdf5的写入逻辑
        }

        strucA_buffer_.clear();
    }
};

class StructBStorager : public DataStorager {
public:
    StructBStorager(const std::string& dir, const std::string& file_type)
            : DataStorager(dir, file_type) {
        data_type_ = "StructB";
    }

    ~StructBStorager() {
        std::lock_guard<std::mutex> lock(mutex_);  // 确保在此过程中没有其他线程尝试写入

        flushBuffer();  // 保存缓冲区数据

        for (auto& [path, file] : csv_open_files_) {
            if (file.is_open()) {
                file.close();  // 关闭文件
            }
        }
        csv_open_files_.clear();  // 清空map
    }

    void asyncWrite(const boost::any& data) override {
        try {
            StructB data_b = boost::any_cast<StructB>(data);
            asyncWrite(data_b);
        } catch (const boost::bad_any_cast& e) {
            std::cout << "Type mismatch: " << e.what() << std::endl;
        }
    }

protected:
    std::vector<StructB> structB_buffer_;

    void asyncWrite(const StructB& data) {
        std::string date = getDateFromTimestamp(data.ns);

        std::lock_guard<std::mutex> lock(mutex_); // 使用互斥锁确保线程安全

        // 在将新数据添加到缓冲区之前，首先检查日期是否发生了变化
        if (last_date_ != "" && last_date_ != date) {
            flushBuffer();  // 如果日期发生了变化，先保存缓冲区的数据
            last_date_ = date;  // 更新日期记录
        }

        // 将新数据添加到缓冲区
        structB_buffer_.push_back(data);

        // 检查缓冲区是否已达到最大大小
        if (structB_buffer_.size() == MAX_BUFFER_SIZE) {
            flushBuffer();  // 当缓冲区满时，保存缓冲区的数据
        }
    }

    std::string formatFloat(double value) {
        std::stringstream ss;
        ss << std::fixed << value;
        std::string s = ss.str();
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }

    void writeToCSV(const std::filesystem::path& p, const std::vector<StructB>& data) {
        if (csv_open_files_.find(p.string()) == csv_open_files_.end()) {
            std::filesystem::create_directories(p.parent_path());
            csv_open_files_[p.string()] = std::ofstream(p, std::ios::app);
        }
        std::ofstream& file = csv_open_files_[p.string()];

        std::stringstream buffer;  // 使用一个字符串流来缓冲数据

        for (const auto& entry : data) {
            buffer << entry.ns << "," << entry.recv_ns << ","
                   << entry.num1 << "," << entry.num2 << entry.num1 << ","
                   << entry.data << ","
                   << entry.total_id << "," << entry.data_id;

            // 移除最后一个逗号
            //buffer.seekp(-1, std::ios_base::cur);
            buffer << "\n";
        }

        file << buffer.str();  // 一次性将所有数据写入文件
        file.flush();          // 确保数据已经写入文件
    }

    void flushBuffer() override {
        // 确定路径和文件名
        StructB& first_data = structB_buffer_[0];
        std::string cur_data = getDateFromTimestamp(first_data.ns);
        std::filesystem::path p = generatePath(cur_data);  // 使用一个函数来生成路径
        printf("generatePath %s\n", p.string().c_str());

        // 如果日期变化，关闭旧的文件句柄
        if (last_date_!= "" && cur_data != last_date_) {
            std::filesystem::path last_path = generatePath(last_date_);
            closeFile(last_path.string(), file_type_);
            last_date_ = cur_data;
        }

        // 根据file_type_决定如何写入
        if (file_type_ == "csv") {
            writeToCSV(p, structB_buffer_);
        } else if (file_type_ == "h5" || file_type_ == "hdf5") {
            // hdf5的写入逻辑
        }

        structB_buffer_.clear();
    }
};
