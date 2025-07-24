#include "generic_host/ConsoleLifecycle.h"

namespace gh {
    ConsoleLifecycle::ConsoleLifecycle(
        const std::shared_ptr<boost::asio::io_context> &io,
        std::shared_ptr<spdlog::logger> logger): IHostLifecycle(std::move(logger) ),
                io_(io),
                signals_(*io, SIGINT, SIGTERM) {
        shutdownFuture_ = shutdownPromise_.get_future().share();
    }

    void ConsoleLifecycle::Start() {
        if (hasStarted_.exchange(true)) {
            // already started, do nothing
            return;
        }

        logger_->info("Start() called. Registering signal handlers...");
        auto self = shared_from_this();  // Keep lifecycle alive during async op
        signals_.async_wait([self](const boost::system::error_code& ec, int signal_number) {
            if (!ec) {
                self->logger_->info("Received signal {}, initiating shutdown...", signal_number);
                self->Shutdown();
            } else {
                self->logger_->error("Signal handler error: {}", ec.message());
            }
        });
    }

    void ConsoleLifecycle::Shutdown() {
        if (!hasStarted_.load()) {
            logger_->warn("Shutdown() called before Start() — ignoring");
            return;
        }

        if (hasStopped_.exchange(true)) {
            // already shut down
            return;
        }

        logger_->info("Shutdown() called. Shutting down...");
        shutdownPromise_.set_value();

        io_->stop();
    }

    int ConsoleLifecycle::Install() {
        logger_->info("Console lifecycle: Install() is a no-op");
        return 0;
    }

    int ConsoleLifecycle::Uninstall() {
        logger_->info("Console lifecycle: Uninstall() is a no-op");
        return 0;
    }

    boost::asio::io_context& ConsoleLifecycle::IoContext() {
        return *io_;
    }

    std::shared_future<void> ConsoleLifecycle::WaitForShutdownAsync() {
        return shutdownFuture_;
    }
}
