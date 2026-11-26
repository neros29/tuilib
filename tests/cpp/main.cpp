#include <gtest/gtest.h>
#include <fstream>
#include "../../src/core/utils/logging.h"

static void log(int level, const std::string& msg) {
    std::string level_str;
    std::ofstream log("logs/gtest.log", std::ios::app);
    switch (level) {
        case LogLevel::INFO:     level_str = "INFO";     break;
        case LogLevel::WARNING:  level_str = "WARNING";  break;
        case LogLevel::ERROR:    level_str = "ERROR";    break;
        case LogLevel::CRITICAL: level_str = "CRITICAL"; break;
        case LogLevel::DEBUG:    level_str = "DEBUG";    break;
        default:                 level_str = "UNKNOWN";  break;
    }
    log << "[" << level_str << "] " << "'" <<msg << "'" << std::endl;
    log.flush();
    log.close();
}

class MyGlobalEnvironment : public ::testing::Environment {
public:
    // This is called before ANY test in any suite starts
    void SetUp() override {
        Logger::set_callback(log);
    }

    // This is called after ALL tests in the entire program finish
    void TearDown() override {
    }
};
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // 3. Register the environment
    // Note: We use 'new' because GTest takes ownership and will delete it
    ::testing::AddGlobalTestEnvironment(new MyGlobalEnvironment);

    return RUN_ALL_TESTS();
}
