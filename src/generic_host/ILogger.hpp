#pragma once
#include <string_view>

namespace generic_host {

    enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };

    struct ILogger {
        virtual void Log(LogLevel level, std::string_view message) = 0;
        virtual ~ILogger() = default;
    };

}