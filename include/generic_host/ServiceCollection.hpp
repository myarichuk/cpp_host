#pragma once
#include <fruit/fruit.h>
#include <functional>
#include <memory>
#include <vector>
#include <spdlog/logger.h>
#include "IHostedService.hpp"

namespace gh {
    class ServiceCollection {
        std::vector<std::function<fruit::Component<>()>> registrations_{};
    public:
        ServiceCollection() = default;

        ServiceCollection(std::vector<std::function<fruit::Component<>()>> regs)
            : registrations_(std::move(regs)) {}

        ServiceCollection AddSingletonInstance(std::shared_ptr<spdlog::logger> logger) const {
            auto regs = registrations_;
            regs.push_back([logger]() {
                return fruit::createComponent()
                    .bindInstance<std::shared_ptr<spdlog::logger>>(logger);
            });
            return ServiceCollection{std::move(regs)};
        }

        template <class T>
        ServiceCollection AddHostedService() const {
            auto regs = registrations_;
            regs.push_back([]() {
                return fruit::createComponent()
                    .addMultibinding<IHostedService, T>();
            });
            return ServiceCollection{std::move(regs)};
        }

        fruit::Component<> buildComponent() const {
            auto comp = fruit::createComponent();
            for (const auto& r : registrations_) {
                comp = comp.install(r());
            }
            return comp;
        }
    };
}
