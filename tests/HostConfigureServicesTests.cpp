#include <catch2/catch_all.hpp>
#include <generic_host/HostBuilder.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/ringbuffer_sink.h>

using namespace gh;

class FooService final : public IHostedService {
    std::shared_ptr<spdlog::logger> _logger;

public:
    explicit FooService(std::shared_ptr<spdlog::logger> logger)
        : _logger(std::move(logger)) { }

    void Start(const boost::asio::io_context&) override {
        _logger->info("FooService started");
    }

    void Stop() override {
        _logger->info("FooService stopped");
    }
};


TEST_CASE("Should log messages when FooService is started and stopped") {
    auto memSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024);
    auto logger = std::make_shared<spdlog::logger>("test_logger", memSink);
    logger->set_level(spdlog::level::info);
    char arg0[] = "--f";
    char* argv[] = { arg0 };
    const auto host = DefaultHostBuilder{1, argv } // empty argc and argv
            .ConfigureServices([&](auto& services) {
            return services.AddSingletonInstance(logger)
                    .template AddHostedService<FooService>();
        })
        .Build();

    std::thread shutdownThread([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        host.Shutdown(); // simulates receiving SIGINT/SIGTERM
    });

    const auto runResult = host.Run();  // this calls FooService::Start()
    shutdownThread.join(); // otherwise we get lovely std::terminate()!

    auto lines = memSink->last_formatted();
    bool hasStart = false, hasStop = false;

    for (auto& msg : lines) {
        if (msg.find("FooService started") != std::string::npos) {
            hasStart = true;
        }
        if (msg.find("FooService stopped") != std::string::npos) {
            hasStop = true;
        }
    }

    REQUIRE(hasStart);
    REQUIRE(hasStop);
}
