#pragma once
#include <boost/di.hpp>
#include <memory>
#include <tuple>
#include <type_traits>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include "IHostedService.hpp"

namespace di = boost::di;

namespace gh {

    namespace detail {

        // detect shared_ptr<T>
        template<typename T>
        struct is_shared_ptr : std::false_type {};

        template<typename T>
        struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

        inline auto create_null_logger() {
            auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            return std::make_shared<spdlog::logger>("null_logger", sink);
        }
    }

    template<class... Binders>
    struct ServiceCollection {
        [[no_unique_address]] std::tuple<Binders...> binds;

        // Interface to Implementation
        template<typename TInterface, typename TImpl>
        requires std::derived_from<TImpl, TInterface>
        constexpr auto AddSingleton() const {
            auto lambda = [] {
                return di::bind<TInterface>.template to<TImpl>().in(di::singleton);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        // Concrete Self-Binding
        template<typename TImpl>
        constexpr auto AddSingleton() const {
            auto lambda = [] {
                return di::bind<TImpl>.in(di::singleton);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        // bind existing instance
        template<typename TImpl>
        constexpr auto AddSingletonInstance(std::shared_ptr<TImpl> instance) const {
            static_assert(!detail::is_shared_ptr<TImpl>::value,
                "Do not pass shared_ptr<shared_ptr<T>>. Just pass shared_ptr<T>.");
            auto lambda = [instance] {
                return di::bind<TImpl>.to(instance);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        // bind existing instance to interface
        template<typename TInterface, typename TImpl>
        requires std::derived_from<TImpl, TInterface>
        constexpr auto AddSingletonInstance(std::shared_ptr<TImpl> instance) const {
            auto lambda = [instance]() {
                return di::bind<TInterface>.to(std::static_pointer_cast<TInterface>(instance));
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        template<typename TService>
        constexpr auto AddHostedService() const {
            auto lambda = [] {
                return di::bind<IHostedService*[]>  //multi-service binding...
                         .template to<TService>()
                         .in(di::singleton);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        template<typename TInterface, typename TImpl>
        requires std::derived_from<TImpl, TInterface>
        constexpr auto AddTransient() const {
            auto lambda = [] {
                return di::bind<TInterface>.template to<TImpl>().in(di::unique);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        template<typename TImpl>
        constexpr auto AddTransient() const {
            auto lambda = [] {
                return di::bind<TImpl>.in(di::unique);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(binds, std::make_tuple(lambda))
            };
        }

        auto Build() const {
            return std::apply([](auto&&... fn) {
                return di::make_injector(fn()...);
            }, binds);
        }
    };


    using Services = ServiceCollection<>;
} // namespace gh
