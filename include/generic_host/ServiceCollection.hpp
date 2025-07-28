#pragma once
#include <boost/di.hpp>
#include <functional>
#include <generic_host/Meta/Typelist.hpp>
#include <generic_host/Meta/LambdaList.hpp>
#include <memory>
#include <tuple>
#include <type_traits>

namespace di = boost::di;

namespace gh {
    struct SingletonScope {};
    struct TransientScope {};

    // Binding<I, Impl, Scope>
    // ReSharper disable once CppTemplateParameterNeverUsed
    template<typename TInterface, typename TImpl, typename TScope = TransientScope>
    struct Binding {
        static_assert(std::is_base_of_v<TInterface, TImpl>,
                      "TImpl must derive from TInterface");

        using Interface = TInterface;
        using Impl = TImpl;
        using Scope = TScope;
    };


    // boost::di integration
    template<typename T>
    struct BoostScope;

    template<>
    struct BoostScope<SingletonScope> {
        using type = di::scopes::singleton;
    };

    template<>
    struct BoostScope<TransientScope> {
        using type = di::scopes::unique;
    };

    // concept for shared_ptr -> kinda C# generics constraint
    template<typename T>
    concept SharedPtr = requires(T t) {
        typename T::element_type;
        requires std::is_same_v<std::remove_cvref_t<T>,
                                std::shared_ptr<typename T::element_type>>;
    };

    template<typename TBinders = types::Typelist<>, typename TFactories = types::Typelist<>>
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

        template<typename TBinding>
        static auto makeMultibinding() {
            using TInterface = typename TBinding::Interface;
            using TImpl = typename TBinding::Impl;
            using TScope = typename TBinding::Scope;
            using ScopeTag = typename BoostScope<TScope>::type;

            if constexpr (std::is_same_v<TInterface, TImpl>) {
                return di::bind<TImpl>.in(ScopeTag{});
            }

            return di::bind<TInterface>.template to<TImpl>().in(ScopeTag{});
        }

        template<typename... Bindings>
        static auto makeInjectorFromBindings(types::Typelist<Bindings...>) {
            return di::make_injector(
                makeMultibinding<Bindings>()...
            );
        }

        template<typename T>
        struct remove_smart_ptr { using type = T; };

        template<typename U>
        struct remove_smart_ptr<std::shared_ptr<U>> { using type = U; };

        template<typename T>
        using remove_smart_ptr_t = typename remove_smart_ptr<T>::type;

        template<typename TResult>
        static auto makeFactoryBinding(std::function<TResult()> factory) {
            using BindT = remove_smart_ptr_t<TResult>; // shared_ptr<Foo> -> Foo
            return di::bind<BindT>.to(std::move(factory));
        }

        // helper to extract result type from std::function
        template<typename TFunc>
        struct factory_result;

        template<typename TResult>
        struct factory_result<std::function<TResult()>> {
            using type = TResult;
        };

        template<typename TList>
        struct InjectorBuilder;

        template<typename... Bs>
        struct InjectorBuilder<types::Typelist<Bs...>> {
            static auto Build(FactoriesTuple& factories) {
                return std::apply([](auto&&... factoryFns) {
                    return di::make_injector(
                        makeMultibinding<Bs>()...,
                        makeFactoryBinding<typename factory_result<std::decay_t<decltype(factoryFns)>>::type>(
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
            return InjectorBuilder<TBinders>::Build(factories);
        }

        template <typename TInterface, SharedPtr TImpl>
        auto AddSingletonAs(TImpl instance) const {
            static_assert(std::is_base_of_v<TInterface, typename TImpl::element_type>, "TImpl must derive from Interface");
            std::function<std::shared_ptr<TInterface>()> newFactory =
                    [instance]() -> std::shared_ptr<TInterface> { return instance; };

            using TNewFactories = typename types::PushBack<std::shared_ptr<TInterface>, TFactories>::Result;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TNewFactories>(std::move(newFactories));
        }

        template <SharedPtr TImpl>
        auto AddSingleton(TImpl instance) const {
            using T = typename TImpl::element_type;
            std::function<std::shared_ptr<T>()> newFactory =
                [instance]() -> std::shared_ptr<T> { return instance; };

            using TNewFactories = typename types::PushBack<std::shared_ptr<T>, TFactories>::Result;
            auto newFactories = concat(factories, lambdas::LambdaList{std::move(newFactory)});
            return ServiceCollection<TBinders, TNewFactories>(std::move(newFactories));
        }

        template <typename TInterface, typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = typename types::PushBack<Binding<TInterface, TImpl>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TFactories>(factories);
        }

        template <typename  TImpl>
        auto AddTransient() const {
            using TNewBinders = typename types::PushBack<Binding<TImpl, TImpl>, TBinders>::Result;
            return ServiceCollection<TNewBinders, TFactories>(factories);
        }
    };

    using Services = ServiceCollection<>;
}
