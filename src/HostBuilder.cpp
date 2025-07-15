#include "generic_host/HostBuilder.hpp"
#include <csignal>
#include <thread>
#include <boost/asio.hpp>

namespace generic_host {

    Host::Host(std::vector<std::shared_ptr<IHostedService>> services)
        : _services(std::move(services)) {}

    int Host::Run() const {
        boost::asio::io_context io;
        boost::asio::signal_set sigs(io, SIGINT, SIGTERM);

        sigs.async_wait([&](auto, int){
            io.stop();
        });

        for (auto& svc : _services)
            svc->Start();

        io.run();          // blocks; CTRL-C calls stop()

        for (auto& svc : _services)
            svc->Stop();

        return 0;
    }

    HostBuilder& HostBuilder::ConfigureServices(std::function<void(std::vector<std::shared_ptr<IHostedService>>&)> configure) {
        configure(_services);
        return *this;
    }

    std::unique_ptr<Host> HostBuilder::Build() {
        return std::make_unique<Host>(_services);
    }

}
