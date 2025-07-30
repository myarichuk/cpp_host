#pragma once
#include <boost/di.hpp>
#include <functional>
#include <generic_host/meta/bindings.hpp>
#include <generic_host/meta/multibindings.hpp>
#include <generic_host/meta/lambdas.hpp>
#include <generic_host/meta/factoryBindings.hpp>
#include <memory>
#include <tuple>
#include <type_traits>
#include <generic_host/IHostedService.hpp>

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

    template<typename TBinders = mp_list<>,
             typename TMultiBinders = mp_list<>,
             typename TFactories = mp_list<>>
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
        struct InjectorBuilder<mp_list<Bs...>, mp_list<MBs...>> {
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

            using TNewFactories = mp_push_back<TFactories, std::shared_ptr<TInterface>>;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TMultiBinders, TNewFactories>(std::move(newFactories));
        }

        template <SharedPtr TImpl>
        auto AddSingleton(TImpl instance) const {
            using T = typename TImpl::element_type;
            std::function<std::shared_ptr<T>()> newFactory =
                [instance]() -> std::shared_ptr<T> { return instance; };

            using TNewFactories = mp_push_back<TFactories, std::shared_ptr<T>>;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TMultiBinders, TNewFactories>(std::move(newFactories));
        }

        template <typename TInterface, typename  TImpl>
        auto AddSingleton() const {
            using TNewBinders = mp_push_back<TBinders, Binding<TInterface, TImpl, SingletonScope>>;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <typename  TImpl>
        auto AddSingleton() const {
            using TNewBinders = mp_push_back<TBinders, Binding<TImpl, TImpl, SingletonScope>>;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <typename TInterface, typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = mp_push_back<TBinders, Binding<TInterface, TImpl>>;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }

        template <HostedService TImpl>
        auto AddHostedService() const {
            return AddMultiTransient<IHostedService, TImpl>();
        }

        template <typename TInterface, typename  TImpl>
        auto AddMultiTransient() const {
            using TNewMultiBinders = mp_push_back<TMultiBinders, Binding<TInterface, TImpl>>;
            return ServiceCollection<TBinders, TNewMultiBinders, TFactories>(factories);
        }

        template <typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = mp_push_back<TBinders, Binding<TImpl, TImpl>>;
            return ServiceCollection<TNewBinders, TMultiBinders, TFactories>(factories);
        }
    };

    using Services = ServiceCollection<>;
}
