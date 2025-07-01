#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>

enum class LogLevel { Info, Warning, Error, Debug };

class Logger {
public:
    Logger(std::ostream& out = std::clog) : out_(out) {}

    void log(const std::string& msg, LogLevel level = LogLevel::Info) {
        std::lock_guard<std::mutex> lock(mutex_);
        out_ << "[" << timestamp() << "] " << to_string(level) << ": " << msg << std::endl;
    }

private:
    std::ostream& out_;
    std::mutex mutex_;

    static std::string to_string(LogLevel level) {
        switch (level) {
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERR";
            case LogLevel::Debug: return "DBG";
        }
        return "UNK";
    }

    static std::string timestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto itt = system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &itt);
#else
        localtime_r(&itt, &tm);
#endif
        char buf[20];
        std::strftime(buf, sizeof(buf), "%F %T", &tm);
        return buf;
    }
};