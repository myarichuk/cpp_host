#include "generic_host/ILogger.hpp"
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>

namespace generic_host {

    class ConsoleLogger final : public ILogger {
    public:
        void Log(const LogLevel level, std::string_view message) override {
            static std::mutex mtx;
            std::lock_guard lock(mtx);

            const char* lvl = nullptr;
            switch (level) {
                case LogLevel::Trace: lvl = "TRACE"; break;
                case LogLevel::Debug: lvl = "DEBUG"; break;
                case LogLevel::Info:  lvl = "INFO "; break;
                case LogLevel::Warn:  lvl = "WARN "; break;
                case LogLevel::Error: lvl = "ERROR"; break;
                case LogLevel::Fatal: lvl = "FATAL"; break;
            }

            auto now = std::time(nullptr);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &now);
#else
            localtime_r(&now, &tm);
#endif
            std::cout << std::put_time(&tm, "%F %T") << " [" << lvl << "] " << message << '\n';
        }
    };

    // TODO: make ConsoleLogger injectable, for now its just compiled and unused.

}
