#include <generic_host/HostBuilder.hpp>
#include <generic_host/IHostedService.hpp>
#include <spdlog/spdlog.h>
#include <memory>

using namespace gh;

/*
class LogIndexerService : public IHostedService {
public:
    void Start() override {
        spdlog::info("LogIndexerService started.");
        // Simulate background tasks
    }

    void Stop() override {
        spdlog::info("LogIndexerService stopped.");
    }
};

int main() {
    HostBuilder builder;
    builder.ConfigureServices([](auto& services) {
        services.push_back(std::make_shared<LogIndexerService>());
    });

    const auto host = builder.Build();
    return host->Run();
}
*/
int main() {
    return 0;
}