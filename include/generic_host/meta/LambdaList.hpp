#pragma once
#include "Typelist.hpp"

namespace gh::lambdas {
    template<typename... Members>
    struct LambdaList {
        std::tuple<Members...> storage;

        explicit LambdaList(Members&&... fs) : storage(std::forward<Members>(fs)...) {}
        explicit LambdaList(std::tuple<Members...>&& t) : storage(std::move(t)) {}

        template<typename F>
        void forEach(F&& mutator) {
            std::apply([&](auto&&... elems) {
                (mutator(elems), ...);
            }, storage);
        }
    };

    template<typename T>
    struct TypeWrapper {
        using type = T;
    };


    template<typename TypeList, typename F>
    struct MakeLambdaListImpl;

    template<typename... Ts, typename F>
    struct MakeLambdaListImpl<types::Typelist<Ts...>, F> {
        static auto make(F&& factory) {
            return LambdaList{factory(TypeWrapper<Ts>{})...};
        }
    };

    template<typename TypeList, typename F>
    auto MakeLambdaList(F&& factory) {
        return MakeLambdaListImpl<TypeList, F>::make(std::forward<F>(factory));
    }

    template<typename... LambdaList1, typename... LambdaList2>
    auto concat(const LambdaList<LambdaList1...>& l1, const LambdaList<LambdaList2...>& l2) {
        return LambdaList<LambdaList1..., LambdaList2...>(
            std::tuple_cat(l1.storage, l2.storage));
    }
}