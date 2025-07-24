#include <boost/asio/signal_set.hpp>
#include <generic_host/IHostLifecycle.h>

namespace gh {
    class ConsoleLifecycle final : public IHostLifecycle,
                                   public std::enable_shared_from_this<ConsoleLifecycle> {
        std::shared_ptr<boost::asio::io_context> io_;
        std::promise<void> shutdownPromise_;
        std::shared_future<void> shutdownFuture_; // for WaitForShutdownAsync()
        boost::asio::signal_set signals_;
        std::atomic_bool hasStopped_{false};
        std::atomic_bool hasStarted_{false};
    public:
        explicit ConsoleLifecycle(
            const std::shared_ptr<boost::asio::io_context> &io,
            std::shared_ptr<spdlog::logger> logger);

        void Start() override;
        void Shutdown() override;
        int Install() override;
        int Uninstall() override;
        boost::asio::io_context & IoContext() override;
        std::shared_future<void> WaitForShutdownAsync() override;
    };
}
