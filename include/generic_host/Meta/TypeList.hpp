#pragma once

namespace gh::types {
    // ReSharper disable once CppTemplateParameterNeverUsed
    template<typename... ListMembers>
    struct Typelist {};

    template<typename TNewMember, typename TExistingList>
    struct PushBack;

    template<typename TNewMember, typename... TExistingList>
    struct PushBack<TNewMember, Typelist<TExistingList...>> {
        using Result = Typelist<TExistingList..., TNewMember>;
    };

    // Alias for PushBack to simplify usage
    template<typename TNew, typename TList>
    using Append = typename PushBack<TNew, TList>::Result;

    template<typename ListMembers>
    struct ForEach;

    template<typename... ListMembers>
    struct ForEach<Typelist<ListMembers...>> {
        template<typename F>
        static void apply(F &&f) {
            (f.template operator()<ListMembers>(), ...);
        }
    };
} // namespace gh::types
