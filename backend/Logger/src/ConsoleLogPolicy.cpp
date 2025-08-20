#include "Utils/Logger/ConsoleLogPolicy.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>

namespace Utils::Logger {
    ConsoleLogPolicy::ConsoleLogPolicy() : stop_(false) {
        thread_ = std::thread(&ConsoleLogPolicy::ProcessQueue, this);
    }

    ConsoleLogPolicy::~ConsoleLogPolicy() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        if (thread_.joinable()) {
            thread_.join();
        }
        while (!message_queue_.empty()) {
            auto msg = message_queue_.front();
            std::cout << FormatMessage(msg.level, msg.message) << std::endl;
            message_queue_.pop();
        }
    }

    void ConsoleLogPolicy::Write(LogLevel level, const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            message_queue_.push({level, message});
        }
        condition_.notify_one();
    }

    void ConsoleLogPolicy::Flush() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout.flush();
    }

    void ConsoleLogPolicy::ProcessQueue() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !message_queue_.empty(); });
            if (stop_ && message_queue_.empty()) {
                return;
            }
            LogMessage message = std::move(message_queue_.front());
            message_queue_.pop();
            lock.unlock();

            std::lock_guard<std::mutex> cout_lock(mutex_);
            std::cout << FormatMessage(message.level, message.message) << std::endl;
        }
    }

    std::string ConsoleLogPolicy::FormatMessage(LogLevel level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto tm = *std::localtime(&now_c);
        std::ostringstream oss;
#ifdef __linux__
        oss << "\033[32m[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << now_ms.count() << "]\033[0m ";
        switch (level) {
            case LogLevel::Info:
                oss << "\033[32m[INFO] \033[0m";
                break;
            case LogLevel::Debug:
                oss << "\033[34m[DEBUG] \033[0m";
                break;
            case LogLevel::Error:
                oss << "\033[31m[ERROR] \033[0m";
                break;
            case LogLevel::Warning:
                oss << "\033[33m[WARNING] \033[0m";
                break;
            case LogLevel::Trace:
                oss << "\033[36m[TRACE] \033[0m";
                break;
    }
#elif _WIN32
        oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << now_ms.count() << "] ";
        switch (level) {
            case LogLevel::Info:
                oss << "[INFO] ";
                break;
            case LogLevel::Debug:
                oss << "[DEBUG] ";
                break;
            case LogLevel::Error:
                oss << "[ERROR] ";
                break;
            case LogLevel::Warning:
                oss << "[WARNING] ";
                break;
            case LogLevel::Trace:
                oss << "[TRACE] ";
                break;
        }
#endif
        oss << message;
        return oss.str();
    }

}