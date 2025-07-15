#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "IHostedService.hpp"
#include "ILogger.hpp"

namespace generic_host {

    class Host {
    public:
        explicit Host(std::vector<std::shared_ptr<IHostedService>> services);
        int Run() const; // blocking call
    private:
        std::vector<std::shared_ptr<IHostedService>> _services;
    };

    class HostBuilder {
    public:
        HostBuilder& ConfigureServices(std::function<void(std::vector<std::shared_ptr<IHostedService>>&)> configure);
        std::unique_ptr<Host> Build();

    private:
        std::vector<std::shared_ptr<IHostedService>> _services;
    };

}