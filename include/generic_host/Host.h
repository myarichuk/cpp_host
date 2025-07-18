#pragma once
#include <memory>
#include <vector>

namespace generic_host {
    struct IHostedService;

    class Host {
    public:
        explicit Host(std::vector<std::shared_ptr<IHostedService>> services);
        int Run() const; // blocking call
    private:
        std::vector<std::shared_ptr<IHostedService>> _services;
    };
}
