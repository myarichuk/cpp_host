#include <catch2/catch_test_macros.hpp>
#include <generic_host/ServiceCollection.hpp>
#include <type_traits>

template<typename A, typename B>
constexpr bool is_same_v = std::is_same<A, B>::value;

struct TypeAsserter {
    template<typename T>
    void operator()() const {
        static_assert(std::is_arithmetic_v<T>, "Only arithmetic types allowed");
    }
};

TEST_CASE("Typelist PushBack and ForEach") {
    using numberTypes = gh::Typelist<int, float>;

    using modifiedNumberTypes =
        gh::PushBack<double, numberTypes>::Result; // added double to the list

    static_assert(is_same_v<
        modifiedNumberTypes,
        gh::Typelist<int, float, double>>, "PushBack failed");

    using SomeTypesList = gh::Typelist<int, float, double>;
    gh::ForEach<SomeTypesList>::apply(TypeAsserter{});
}

TEST_CASE("LambdaList forEach runs all lambdas ") {
    bool a = false, b = false, c = false;

    gh::LambdaList list(
        [&] { a = true; },
        [&] { b = true; },
        [&] { c = true; }
    );

    list.forEach([](auto& f) { f(); });

    REQUIRE(a);
    REQUIRE(b);
    REQUIRE(c);
}

TEST_CASE("LambdaList pushBack merges two lambda lists") {
    int x = 0;

    const gh::LambdaList l1([&x] { x += 1; }, [&x] { x += 2; }) ;
    const gh::LambdaList l2([&x] { x += 3; });

    auto merged = gh::pushBack(l1, l2);

    merged.forEach([](auto& f) { f(); });

    REQUIRE(x == 6);
}