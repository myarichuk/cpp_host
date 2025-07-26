#include <catch2/catch_all.hpp>
#include <generic_host/HostBuilder.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <boost/asio/steady_timer.hpp>
/*
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

class HeartbeatService final : public IHostedService {
    std::shared_ptr<spdlog::logger> _logger;
    std::unique_ptr<boost::asio::steady_timer> _timer;
    bool _stopped = false;

public:
    explicit HeartbeatService(std::shared_ptr<spdlog::logger> logger)
        : _logger(std::move(logger)) {}

    void Start(const boost::asio::io_context& io) override {
        _timer = std::make_unique<boost::asio::steady_timer>(const_cast<boost::asio::io_context&>(io));
        scheduleNext();
        _logger->info("HeartbeatService started");
    }

    void Stop() override {
        _stopped = true;
        _logger->info("HeartbeatService stopped");
    }

private:
    void scheduleNext() {
        if (_stopped) return;

        _timer->expires_after(std::chrono::milliseconds(500));
        _timer->async_wait([this](const boost::system::error_code& ec) {
            if (!ec && !_stopped) {
                _logger->info("heartbeat...");
                scheduleNext();
            }
        });
    }
};

class PingService final : public IHostedService {
    std::shared_ptr<spdlog::logger> _logger;
    std::unique_ptr<boost::asio::steady_timer> _timer;
    bool _stopped = false;

public:
    explicit PingService(std::shared_ptr<spdlog::logger> logger)
        : _logger(std::move(logger)) {}

    void Start(const boost::asio::io_context& io) override {
        _timer = std::make_unique<boost::asio::steady_timer>(const_cast<boost::asio::io_context&>(io));
        scheduleNext();
        _logger->info("PingService started");
    }

    void Stop() override {
        _stopped = true;
        _logger->info("PingService stopped");
    }

private:
    void scheduleNext() {
        if (_stopped) return;

        _timer->expires_after(std::chrono::milliseconds(750));
        _timer->async_wait([this](const boost::system::error_code& ec) {
            if (!ec && !_stopped) {
                _logger->info("ping");
                scheduleNext();
            }
        });
    }
};

TEST_CASE("Should run multiple async services concurrently") {
    auto memSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024);
    auto logger = std::make_shared<spdlog::logger>("test_logger", memSink);
    logger->set_level(spdlog::level::info);
    char arg0[] = "--f";
    char* argv[] = { arg0 };

    const auto host = DefaultHostBuilder{1, argv}
    .ConfigureServices([&](auto& services) {
        return services.AddSingletonInstance(logger)
                       .template AddHostedService<HeartbeatService>()
                       .template AddHostedService<PingService>();
    })
    .Build();

    std::thread shutdownThread([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        host.Shutdown();
    });

    const auto runResult = host.Run();
    shutdownThread.join();

    auto lines = memSink->last_formatted();
    int heartbeatCount = 0;
    int pingCount = 0;

    for (auto& msg : lines) {
        if (msg.find("heartbeat...") != std::string::npos) heartbeatCount++;
        if (msg.find("ping") != std::string::npos) pingCount++;
    }

    REQUIRE(heartbeatCount >= 5);
    REQUIRE(pingCount >= 3);
}
*/