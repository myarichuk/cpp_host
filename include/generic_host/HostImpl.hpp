#pragma once
#include <boost/asio/signal_set.hpp>
#include "IHostedService.hpp"
#include "IHostLifecycle.h"
#include "ServiceCollection.hpp"
#include <fruit/fruit.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

namespace gh {
    namespace detail {
        inline auto create_null_logger() {
            auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            return std::make_shared<spdlog::logger>("null_logger", sink);
        }
    }


    template<class ServiceList>
    class HostImpl {
        ServiceList services_;
        std::shared_ptr<IHostLifecycle> lifecycle_;
        std::shared_ptr<boost::asio::io_context> io_;

        [[nodiscard]] auto buildInjector() const {
            auto comp = services_.buildComponent();
            return fruit::Injector<>(comp);
        }

    public:
        explicit HostImpl(
            const std::shared_ptr<boost::asio::io_context> &io,
            ServiceList services,
            std::shared_ptr<IHostLifecycle> lifecycle) :
            services_(std::move(services)), lifecycle_(std::move(lifecycle)), io_(io) {}

        void Shutdown() const {
            lifecycle_->Shutdown();
        }

        [[nodiscard]] int Run() const {
            auto injector = buildInjector();
            auto services = injector.getMultibindings<IHostedService*>();

            lifecycle_->Start();

            for (auto serviceInstance : services) {
                serviceInstance->Start(*io_);
            }

            lifecycle_->WaitForShutdownAsync().wait();

            for (auto serviceInstance : services) {
                serviceInstance->Stop();
            }

            return 0;
        }
    };
}
