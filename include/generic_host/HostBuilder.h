#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <generic_host/IHostedService.hpp>
#include "Host.h"

namespace generic_host {

    class HostBuilder {
    public:
        HostBuilder& ConfigureServices(std::function<void(std::vector<std::shared_ptr<IHostedService>>&)> configure);
        std::unique_ptr<Host> Build();

    private:
        std::vector<std::shared_ptr<IHostedService>> _services;
    };

}
