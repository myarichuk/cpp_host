#pragma once

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