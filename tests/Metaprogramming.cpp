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
    using numberTypesList = gh::Typelist<int, float>;

    using modifiedNumberTypesList =
        gh::PushBack<double, numberTypesList>::type; // added double to the list

    static_assert(is_same_v<
        modifiedNumberTypesList,
        gh::Typelist<int, float, double>>, "PushBack failed");

    using SomeTypesList = gh::Typelist<int, float, double>;
    gh::ForEach<SomeTypesList>::apply(TypeAsserter{});
}

TEST_CASE("LambdaList runs all lambdas") {
    bool a = false, b = false, c = false;

    gh::LambdaList list(
        [&]() { a = true; },
        [&]() { b = true; },
        [&]() { c = true; }
    );

    list.forEach([](auto& f) { f(); });

    REQUIRE(a);
    REQUIRE(b);
    REQUIRE(c);
}

TEST_CASE("LambdaList pushBack merges") {
    int x = 0;

    gh::LambdaList l1([&]() { x += 1; });
    gh::LambdaList l2([&]() { x += 2; });

    auto merged = gh::pushBack(l1, l2);

    merged.forEach([](auto& f) { f(); });

    REQUIRE(x == 3);
}