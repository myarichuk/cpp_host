#pragma once
#include "IHostLifecycle.h"
#include "ServiceCollection.hpp"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>

namespace gh {
/*
    template<class F>
    auto configureServices(F&& f) {
        auto services =
            Services{}.AddSingletonInstance(detail::create_null_logger());  // using Services = ServiceCollection<>
        f(services);                 // mutates it
        return services;            // return the modified collection
    }
*/
    template<class ServiceList>
    class HostImpl {
        ServiceList services_;
        std::shared_ptr<IHostLifecycle> lifecycle_;
        std::shared_ptr<boost::asio::io_context> io_;

        /*
        [[nodiscard]] auto buildDI() const
        {
            return std::apply(
                [](auto&&... f){ return di::make_injector(f()...); },
                services_.binds);
        }
        */
    public:
        explicit HostImpl(
            const std::shared_ptr<boost::asio::io_context> &io,
            ServiceList services,
            std::shared_ptr<IHostLifecycle> lifecycle) :
            services_(std::move(services)), lifecycle_(std::move(lifecycle)), io_(io) {}

        void Shutdown() const {
            lifecycle_->Shutdown();
        }

        [[nodiscard]] int Run() const
        {
/*
            auto di = buildDI();

            // resolve all registered IHostedService's
            auto services = di.template create<std::vector<std::shared_ptr<IHostedService>>>();

            lifecycle_->Start();

            for(const auto serviceInstance : services){
                serviceInstance->Start(*io_);
            }

            lifecycle_->WaitForShutdownAsync().wait();

            for(const auto serviceInstance : services){
                serviceInstance->Stop();
            }
*/
            return 0;
        }
    };
}
