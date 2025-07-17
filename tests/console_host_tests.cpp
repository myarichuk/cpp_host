#include <catch2/catch_all.hpp>
#include <generic_host/ConsoleHost.h>

using namespace generic_host;

class MockLogger : public ILogger {
public:
    struct LogEntry {
        LogLevel level;
        std::string message;
    };

    std::vector<LogEntry> logs;

    void Log(const LogLevel level, std::string_view message) override {
        logs.push_back({level, std::string(message)});
    }

    void Clear() {
        logs.clear();
    }

    bool ContainsMessage(const LogLevel level, const std::string &substring) const {
        for (const auto&[level, message] : logs) {
            if (level == level && message.find(substring) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
};


TEST_CASE("ConsoleHost should properly execute services") {
    auto logger = std::make_shared<MockLogger>();
    ConsoleHost host(logger);

    bool appWasExecuted = false;
    auto startApp = [&]() {
        appWasExecuted = true;
    };

    int result = host.Run(startApp);

    REQUIRE(result == 0);
    REQUIRE(appWasExecuted);
    REQUIRE(logger->ContainsMessage(LogLevel::Info, "Console Host starting"));
    REQUIRE(logger->ContainsMessage(LogLevel::Info, "Console Host stopped"));
}

TEST_CASE("ConsoleHost should properly handle exceptions") {
    auto logger = std::make_shared<MockLogger>();
    ConsoleHost host(logger);

    auto startApp = []() {
        throw std::runtime_error("Foobar!");
    };

    int result = host.Run(startApp);

    REQUIRE(result == 1);
    REQUIRE(logger->ContainsMessage(LogLevel::Error, "Console Host failed: Foobar!"));
    REQUIRE(logger->ContainsMessage(LogLevel::Error, "Stack trace:"));
}

TEST_CASE("ConsoleHost should properly handle unknown exceptions") {
    auto logger = std::make_shared<MockLogger>();
    ConsoleHost host(logger);

    auto startApp = []() {
        throw 42; // non-standard exception
    };

    int result = host.Run(startApp);

    REQUIRE(result == 1);
    REQUIRE(logger->ContainsMessage(LogLevel::Fatal, "Console Host failed with unknown exception"));
    REQUIRE(logger->ContainsMessage(LogLevel::Fatal, "Stack trace:"));
}
