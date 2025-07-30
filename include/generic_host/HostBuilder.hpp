#pragma once
#include <memory>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <generic_host/ConsoleLifecycle.h>
#include <generic_host/HostImpl.hpp>
#include <generic_host/IHostLifecycle.h>

namespace gh {
    namespace detail {
        inline auto create_null_logger() {
            auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            return std::make_shared<spdlog::logger>("null_logger", sink);
        }
    }
    class Host;

    template<typename ServiceList>
    class HostBuilder {
        template<typename> friend class HostBuilder;
        ServiceList services_;
        CLI::App app_;
        std::shared_ptr<boost::asio::io_context> io_ = std::make_shared<boost::asio::io_context>();
        std::shared_ptr<spdlog::logger> logger_ = nullptr;
    public:
        explicit HostBuilder(int argc, char** argv)
         : HostBuilder("Generic Host", argc, argv) { }
        
        explicit HostBuilder(const std::string& hostDescription, int argc, char** argv)
            : services_{}, app_{ hostDescription } {

            //TODO: properly add subcommands here

            try {
                app_.parse(argc, argv);
            } catch (const CLI::ParseError& e) {
                std::exit(app_.exit(e));
            }
        }

        template<typename F>
        auto ConfigureServices(F&& f) const {
            auto updated = configureServices(std::forward<F>(f), services_);
            return HostBuilder<decltype(updated)>{ std::move(updated) };
        }

        [[nodiscard]] auto Build() {
            /*
            if (*install) {  }
            if (*uninstall) {  }
            if (*run) {  }
             */
            // for now hardcode to console lifecycle
            std::shared_ptr<IHostLifecycle> hostLifecycle;
            if (!logger_) {
                logger_ = detail::create_null_logger();
            }

            // TODO: make this proper (instantiate by flags, OS etc), for now hardcode
            const auto lifecycle = std::make_shared<ConsoleLifecycle>(io_, logger_);

            return HostImpl(io_, services_, lifecycle);
        }

    private:
        explicit HostBuilder(ServiceList services)
            : services_{ std::move(services) } {}

        template<typename F, typename S>
        static auto configureServices(F&& f, S const& existing) {
            auto result = f(existing); // must return updated ServiceCollection

            static_assert(!std::is_void_v<decltype(result)>,
                "Service configuration lambda must return the modified ServiceCollection");

            return result;
        }
    };

    using DefaultHostBuilder = HostBuilder<Services>;
}
