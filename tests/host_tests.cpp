#define CATCH_CONFIG_MAIN
#include <csignal>
#include <thread>
#include <catch2/catch_all.hpp>
#include <generic_host/HostBuilder.hpp>

using namespace generic_host;

class DummyService final : public IHostedService {
public:
    bool started = false;
    bool stopped = false;

    void Start() override { started = true; }
    void Stop() override { stopped = true; }
};

TEST_CASE("Host should successfully start and stops services") {
    auto service = std::make_shared<DummyService>();

    HostBuilder builder;
    builder.ConfigureServices([&service](auto& services) {
        services.push_back(service);
    });

    auto host = builder.Build();

    std::thread t([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::raise(SIGINT); // Simulate Ctrl+C
    });

    int result = host->Run();
    t.join();

    REQUIRE(result == 0);
    REQUIRE(service->started);
    REQUIRE(service->stopped);
}
