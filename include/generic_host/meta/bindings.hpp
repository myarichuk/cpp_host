#pragma once
#include <boost/di.hpp>
#include <boost/mp11/algorithm.hpp>

namespace di = boost::di;
using namespace boost::mp11;

namespace gh::boost_helpers {
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

    template<typename TBinding>
    static auto makeTypeBindings() {
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
    static auto makeInjectorFromBindings(mp_list<Bindings...>) {
        return di::make_injector(
            makeTypeBindings<Bindings>()...
        );
    }
}