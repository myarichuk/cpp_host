#pragma once
#include <functional>
#include <memory>
#include <boost/di.hpp>
#include <boost/mp11/algorithm.hpp>

namespace di = boost::di;
using namespace boost::mp11;

namespace gh::boost_helpers {
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

    // helper to extract a result type from std::function
    template<typename TFunc>
    struct factory_result;

    template<typename TResult>
    struct factory_result<std::function<TResult()>> {
        using type = TResult;
    };
}
