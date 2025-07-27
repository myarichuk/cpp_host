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

    template<typename... Ts>
   struct Typelist {};

    template<typename T, typename TL>
    struct PushBack;

    template<typename T, typename... Ts>
    struct PushBack<T, Typelist<Ts...>> {
        using type = Typelist<Ts..., T>;
    };

    template<typename TL>
    struct ForEach;

    template<typename... Ts>
    struct ForEach<Typelist<Ts...>> {
        template<typename F>
        static void apply(F&& f) {
            (f.template operator()<Ts>(), ...);
        }
    };

    template<typename... Fs>
    struct LambdaList {
        std::tuple<Fs...> storage;

        explicit LambdaList(Fs&&... fs) : storage(std::forward<Fs>(fs)...) {}
        explicit LambdaList(std::tuple<Fs...>&& t) : storage(std::move(t)) {}

        template<typename F>
        void forEach(F&& f) {
            std::apply([&](auto&&... elems) {
                (f(elems), ...);
            }, storage);
        }
    };

    template<typename... Fs1, typename... Fs2>
    auto pushBack(const LambdaList<Fs1...>& l1, const LambdaList<Fs2...>& l2) {
        return LambdaList<Fs1..., Fs2...>(std::tuple_cat(l1.storage, l2.storage));
    }


    /* ----------------------------------------- */
    // Actual service collection
    /* ----------------------------------------- */

    template<class... Binders>
    class ServiceCollection {
    };

    using Services = ServiceCollection<>;
} // namespace gh
