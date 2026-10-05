#pragma once
#include <iostream>
#include <functional>

 namespace LogLevel {
        inline constexpr int DEBUG    = 10;
        inline constexpr int INFO     = 20;
        inline constexpr int WARNING  = 30;
        inline constexpr int ERROR    = 40;
        inline constexpr int CRITICAL = 50;
    }

class Logger {
public:
    // We use std::function to hold the Python function
    using LogCallback = std::function<void(int, const std::string&)>;

    static void set_callback(LogCallback cb) {
        callback = cb;
    }

    static void log(int level, const std::string& msg) {
        if (callback) {
            callback(level, msg);
        } else {
            fallback(level, msg);
        }
    }

private:
    static void fallback(int level, const std::string& msg) {
        std::string level_str;
        switch (level) {
            case LogLevel::INFO:     level_str = "INFO";     break;
            case LogLevel::WARNING:  level_str = "WARNING";  break;
            case LogLevel::ERROR:    level_str = "ERROR";    break;
            case LogLevel::CRITICAL: level_str = "CRITICAL"; break;
            case LogLevel::DEBUG:    level_str = "DEBUG";    break;
            default:                 level_str = "UNKNOWN";  break;
        }
        std::clog << "[" << level_str << "] " << msg << std::endl;
    }
    inline static LogCallback callback;
};

// The Public API (This is what your pybind11 module will expose)
namespace logger {
    inline void debug(const std::string& msg) {
        Logger::log(LogLevel::DEBUG, msg);
    }

    inline void info(const std::string& msg) {
        Logger::log(LogLevel::INFO, msg);
    }

    inline void warn(const std::string& msg) {
        Logger::log(LogLevel::WARNING, msg);
    }

    inline void error(const std::string& msg) {
        Logger::log(LogLevel::ERROR, msg);
    }

    inline void critical(const std::string& msg) {
        Logger::log(LogLevel::CRITICAL, msg);
    }
}
