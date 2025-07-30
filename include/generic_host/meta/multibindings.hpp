#pragma once
#include <boost/mp11.hpp>
#include <boost/mp11/bind.hpp>
#include "bindings.hpp"

using namespace boost::mp11;
namespace gh::boost_helpers {
    template<class TInterface, class TImplList>
    struct MultiBindingGroup
    {
        using Interface = TInterface;
        using Impls = TImplList;   // mp_list<Impl1, Impl2, ...>
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

    // mp_list<mp_list<I1, ImplA>, mp_list<I1, ImplB>, mp_list<I2, ImplC>, ...>
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
}