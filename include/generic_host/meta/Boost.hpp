#pragma once
#include <boost/di.hpp>
#include <functional>
#include <memory>
#include <boost/mp11/list.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/mp11/algorithm.hpp>
#include "Typelist.hpp"

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

    template<class I, class ImplList>
    struct MultiBindingGroup
    {
        using Interface = I;
        using Impls = ImplList;   // mp_list<Impl1, Impl2, ...>
    };

    /*
    template<class Group, class ScopeTag>
    auto makeMultibinding() {
        using I     = typename Group::Interface;
        using Impls = typename Group::Impls; // mp_list<Impl1,Impl2,…>

        return mp_apply(
            [&](auto... ImplTs){
                return di::bind<I*[]>.template to<ImplTs...>().in(ScopeTag{});
            }, Impls{});
    }
    */

    template<typename... Bindings>
    static auto makeInjectorFromBindings(types::Typelist<Bindings...>) {
        return di::make_injector(
            makeTypeBindings<Bindings>()...
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
}