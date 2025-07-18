#include <generic_host/HostBuilder.h>
#include <generic_host/Host.h>
#include <boost/asio.hpp>

namespace generic_host {
    HostBuilder& HostBuilder::ConfigureServices(std::function<void(std::vector<std::shared_ptr<IHostedService>>&)> configure) {
        configure(_services);
        return *this;
    }

    std::unique_ptr<Host> HostBuilder::Build() {
        return std::make_unique<Host>(_services);
    }

}
