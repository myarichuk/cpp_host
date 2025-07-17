
#include <string>
#include <spdlog/spdlog.h>
#include <boost/stacktrace.hpp>
#include <generic_host/ConsoleHost.h>
#include <generic_host/IPlatformHost.h>

using namespace generic_host;

ConsoleHost::ConsoleHost(std::shared_ptr<ILogger> logger): IPlatformHost(std::move(logger)) {}

int ConsoleHost::Run(const std::function<void()> startApp) {
    try {
        _logger->Log(LogLevel::Info, "Console Host starting");
        startApp();
        _logger->Log(LogLevel::Info, "Console Host stopped");
        return 0;
    }
    catch(const std::exception& ex) {
        std::stringstream ss;
        ss << "Console Host failed: " << ex.what() << "\n"
                << "Stack trace:\n" << boost::stacktrace::stacktrace();

        _logger->Log(LogLevel::Error, ss.str());
        return 1;
    }
    catch(...) {
        std::stringstream ss;
        ss << "Console Host failed with unknown exception\n"
                << "Stack trace:\n" << boost::stacktrace::stacktrace();

        _logger->Log(LogLevel::Fatal, ss.str());
        return 1;
    }
}
