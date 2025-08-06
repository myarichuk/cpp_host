#pragma once
#include <boost/mp11.hpp>
#include <boost/mp11/bind.hpp>
#include <generic_host/meta/bindings.hpp>
#include <boost/mp11/algorithm.hpp>

using namespace boost::mp11;
namespace gh::boost_helpers {
    template<class TInterface, class TImplList>
    struct MultiBindingGroup
    {
        using Interface = TInterface;
        using Implementations = TImplList;   // mp_list<Impl1, Impl2, ...>
    };

    // trait to detect specialization (default case -> false_type, correct specialization -> true_type)
    template<typename T>
    struct is_binding_specialization : std::false_type {};

    template<typename TInterface, typename TImpl, typename TScope>
    struct is_binding_specialization<Binding<TInterface, TImpl, TScope>> : std::true_type {};

    template<typename TInterface, typename TImpl>
    struct is_binding_specialization<Binding<TInterface, TImpl>> : std::true_type {};

    template<typename T>
    concept BindingType = is_binding_specialization<T>::value;

    template<typename Binding> using InterfaceOf = typename Binding::Interface;
    template<typename Binding> using ImplOf = typename Binding::Impl;

    template<typename Binding>
    using KeyValue = mp_list<InterfaceOf<Binding>, ImplOf<Binding>>;

    template<typename BindingList>
    using KVList = mp_transform<KeyValue, BindingList>;

    template<typename BindingList>
    using UniqueInterfaces = mp_unique<mp_transform<InterfaceOf, BindingList>>;

    template<class TInterface, class TBindingList>
    using ImplListFor = mp_transform<
            mp_second,
            mp_filter_q<
                //predicate: InterfaceOf<Binding> == TInterface
                mp_bind<
                    std::is_same,
                    mp_bind<InterfaceOf, _1>,   // defer InterfaceOf
                    TInterface>,
                TBindingList>
        >;

    // (interface + implementations) -> MultiBindingGroup
    template<class BindingList, class Interface>
    struct GroupForInterface
    {
        using type = MultiBindingGroup<
            Interface,
            mp_transform<
                ImplOf,
                mp_filter_q<
                    // ️predicate (for the filter)
                    mp_bind<
                        std::is_same,
                        mp_bind<InterfaceOf, _1>,
                        Interface>,
                    //list
                    BindingList>
            >
        >;
    };


    template<class BindingList>
    using GroupedBindings =
        mp_transform_q<
            mp_bind_front_q<
                mp_quote_trait<GroupForInterface>,
                BindingList>,
            UniqueInterfaces<BindingList>
        >;

    template<class Group, class ScopeTag>
    constexpr auto makeMultibinding() {
        using Interface = typename Group::Interface;
        using ImplList  = typename Group::Implementations;

        auto lambda = []<typename... Impls>(mp_list<Impls...>) {
            if constexpr (std::is_same_v<ScopeTag, boost::di::scopes::unique>) {
                return boost::di::bind<Interface*[]>
                    .template to<Impls...>();
            } else {
                return boost::di::bind<Interface*[]>
                    .template to<Impls...>()
                    .in(ScopeTag{});
            }
        };
        return lambda(ImplList{});
    }
}