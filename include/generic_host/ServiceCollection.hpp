#pragma once
#include <boost/di.hpp>
#include <tuple>
#include <type_traits>
#include <spdlog/sinks/null_sink.h>
#include "IHostedService.hpp"

namespace di = boost::di;

namespace gh {
    /* ----------------------------------------- */
    // Metaprogramming crap
    /* ----------------------------------------- */

    template<typename... ListMembers>
    struct Typelist {};

    template<typename TNewMember, typename TExistingList>
    struct PushBack;

    template<typename TNewMember, typename... TExistingList>
    struct PushBack<TNewMember, Typelist<TExistingList...>> {
        using Result = Typelist<TExistingList..., TNewMember>; //define PushBack resulting type
    };

    template<typename ListMembers>
    struct ForEach;

    template<typename... ListMembers>
    struct ForEach<Typelist<ListMembers...>> {
        template<typename F>
        static void apply(F&& f) {
            (f.template operator()<ListMembers>(), ...);
        }
    };

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

    template<typename... LambdaList1, typename... LambdaList2>
    auto pushBack(const LambdaList<LambdaList1...>& l1, const LambdaList<LambdaList2...>& l2) {
        return LambdaList<LambdaList1..., LambdaList2...>(
            std::tuple_cat(l1.storage, l2.storage));
    }


    /* ----------------------------------------- */
    // Actual service collection
    /* ----------------------------------------- */

    template<class... Binders>
    class ServiceCollection {
    };

    using Services = ServiceCollection<>;
} // namespace gh
