
#pragma once
#include <functional>
#include <memory>
#include "ILogger.hpp"

namespace generic_host {
    class IPlatformHost {
    public:
        virtual int Run(std::function<void()> startApp) = 0;
        virtual ~IPlatformHost() = default;

        explicit IPlatformHost(std::shared_ptr<ILogger> logger) : _logger(std::move(logger)) {}

    protected:
        std::shared_ptr<ILogger> _logger;
    };
}