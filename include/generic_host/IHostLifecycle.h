#pragma once
#include <future>  // for std::future<void>
#include <boost/asio/io_context.hpp>
#include <spdlog/logger.h>

namespace gh {

    // Purpose: Cross-platform abstraction for managing host lifetime
    // Windows Service, Linux Daemon, Mac launchd, or Console App
    class IHostLifecycle {
    protected:
        std::shared_ptr<spdlog::logger> logger_;
    public:
        virtual void Start() = 0;
        virtual void Shutdown() = 0;
        virtual int Install() = 0;
        virtual int Uninstall() = 0;

        virtual boost::asio::io_context& IoContext() = 0;
        virtual std::shared_future<void> WaitForShutdownAsync() = 0;

        explicit IHostLifecycle(std::shared_ptr<spdlog::logger> logger): logger_(std::move(logger)) {}
        virtual ~IHostLifecycle() = default;
    };
}
