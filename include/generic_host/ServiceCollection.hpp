#pragma once
#include <boost/di.hpp>
#include <memory>
#include <tuple>

namespace di = boost::di;

namespace generic_host {

    template<class... Binders>
    struct ServiceCollection {
        std::tuple<Binders...> _binds;

        // AddSingleton<TInterface, TImpl>
        template<typename TInterface, typename TImpl>
        constexpr auto AddSingleton() const {
            static_assert(std::derived_from<TImpl, TInterface>);
            auto lambda = [] {
                return di::bind<TInterface>.template to<TImpl>().in(di::singleton);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        template<typename TInterface, typename TImpl>
        constexpr auto AddTransient() const {
            static_assert(std::derived_from<TImpl, TInterface>);
            auto lambda = [] {
                return di::bind<TInterface>.template to<TImpl>().in(di::unique);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        template<typename TImpl>
        constexpr auto AddSingleton() const {
            auto lambda = [] {
                return di::bind<TImpl>.in(di::singleton);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        template<typename TImpl>
        constexpr auto AddTransient() const {
            auto lambda = [] {
                return di::bind<TImpl>.in(di::unique);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        template<typename TImpl>
        constexpr auto AddSingleton(std::shared_ptr<TImpl> instance) const {
            auto lambda = [instance] {
                return di::bind<TImpl>.to(instance);
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        template<typename TInterface, typename TImpl>
        constexpr auto AddSingleton(std::shared_ptr<TImpl> instance) const {
            static_assert(std::derived_from<TImpl, TInterface>);

            auto lambda = [instance]() {
                return di::bind<TInterface>.to(std::static_pointer_cast<TInterface>(instance));
            };
            return ServiceCollection<Binders..., decltype(lambda)>{
                std::tuple_cat(_binds, std::make_tuple(lambda))
            };
        }

        auto Build() const {
            return std::apply([](auto&&... fn) {
                return di::make_injector(fn()...);
            }, _binds);
        }
    };

    using Services = ServiceCollection<>;

}