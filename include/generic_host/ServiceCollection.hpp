#pragma once
#include <boost/di.hpp>
#include <functional>
#include <generic_host/meta/Boost.hpp>
#include <generic_host/meta/LambdaList.hpp>
#include <generic_host/meta/Typelist.hpp>
#include <memory>
#include <tuple>
#include <type_traits>

#include "IHostedService.hpp"

namespace di = boost::di;
using namespace gh::boost_helpers;

namespace gh {
    // concept for shared_ptr -> kinda C# generics constraint
    template<typename TPtr>
    concept SharedPtr = requires(TPtr t) {
        typename TPtr::element_type;
        requires std::is_same_v<std::remove_cvref_t<TPtr>,
                                std::shared_ptr<typename TPtr::element_type>>;
    };

    template<typename TImpl>
    concept HostedService = std::is_base_of_v<IHostedService, std::remove_cvref_t<TImpl>>;

    template<typename TBinders = types::Typelist<>,
             typename TMultiBinders = types::Typelist<>,
             typename TFactories = types::Typelist<>>
    class ServiceCollection {
        // build lambda list types from TFactories types
        using FactoriesTuple = decltype(lambdas::MakeLambdaList<TFactories>(
            []<typename TWrapper> (TWrapper) {
                using T = typename TWrapper::type;
                return std::function<T()>{};
            }
        ));

        // create a LambdaList<...> on the fly from TFactories
        FactoriesTuple factories;

        template<typename TBindingList, typename TMultiBindingList>
        struct InjectorBuilder;

        template<typename... Bs, typename...  MBs>
        struct InjectorBuilder<types::Typelist<Bs...>, types::Typelist<MBs...>> {
            static auto Build(FactoriesTuple& factories) {
                return std::apply([](auto&&... factoryFns) {
                    return di::make_injector(
                        makeTypeBindings<Bs>()...,
                        makeFactoryBinding<
                            typename factory_result<std::decay_t<decltype(factoryFns)>>::type>(
                                std::forward<decltype(factoryFns)>(factoryFns)
                        )...
                    );
                }, factories.storage);
            }
        };
    public:
#ifdef UNIT_TEST
        template<std::size_t Index>
        auto& GetFactory() const {
            return std::get<Index>(factories.storage);
        }
#endif

        ServiceCollection() = default;
        explicit ServiceCollection(FactoriesTuple f) : factories(std::move(f)) {}

        auto Build() {
            return InjectorBuilder<TBinders, TMultiBinders>::Build(factories);
        }

        template <typename TInterface, SharedPtr TImpl>
        auto AddSingletonAs(TImpl instance) const {
            static_assert(std::is_base_of_v<TInterface, typename TImpl::element_type>, "TImpl must derive from Interface");
            std::function<std::shared_ptr<TInterface>()> newFactory =
                    [instance]() -> std::shared_ptr<TInterface> { return instance; };

            using TNewFactories = typename types::PushBack<std::shared_ptr<TInterface>, TFactories>::Result;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TMultiBinders, TNewFactories>(std::move(newFactories));
        }

        template <SharedPtr TImpl>
        auto AddSingleton(TImpl instance) const {
            using T = typename TImpl::element_type;
            std::function<std::shared_ptr<T>()> newFactory =
                [instance]() -> std::shared_ptr<T> { return instance; };

            using TNewFactories = typename types::PushBack<std::shared_ptr<T>, TFactories>::Result;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TMultiBinders, TNewFactories>(std::move(newFactories));
        }

        template <typename TInterface, typename  TImpl>
        auto AddSingleton() const {
            using TNewBinders = typename types::PushBack<Binding<TInterface, TImpl, SingletonScope>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <typename  TImpl>
        auto AddSingleton() const {
            using TNewBinders = typename types::PushBack<Binding<TImpl, TImpl, SingletonScope>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <typename TInterface, typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = typename types::PushBack<Binding<TInterface, TImpl>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <HostedService TImpl>
        auto AddHostedService() const {
            return AddMultiTransient<TImpl>();
        }

        template <typename TInterface, typename  TImpl>
        auto AddMultiTransient() const {
            using TNewMultiBinders = typename types::PushBack<Binding<TInterface, TImpl>, TMultiBinders>::Result;
            return ServiceCollection<TBinders, TNewMultiBinders, TFactories>(factories);
        }

        template <typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = typename types::PushBack<Binding<TImpl, TImpl>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }
    };

    using Services = ServiceCollection<>;
}
