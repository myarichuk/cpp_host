#include <catch2/catch_test_macros.hpp>
#include <generic_host/ServiceCollection.hpp>
#include <type_traits>
#include <generic_host/Meta/TypeList.hpp>
#include <generic_host/Meta/LambdaList.hpp>

struct TypeAsserter {
    template<typename T>
    void operator()() const {
        static_assert(std::is_arithmetic_v<T>, "Only arithmetic types allowed");
    }
};

using namespace gh::types;
using namespace gh::lambdas;

TEST_CASE("Typelist PushBack and ForEach") {
    using numberTypes = Typelist<int, float>;

    using modifiedNumberTypes =
        PushBack<double, numberTypes>::Result; // added double to the list

    static_assert(std::is_same_v<
        modifiedNumberTypes,
        Typelist<int, float, double>>, "PushBack failed");

    using SomeTypesList = Typelist<int, float, double>;
    ForEach<SomeTypesList>::apply(TypeAsserter{});
}

TEST_CASE("LambdaList forEach runs all lambdas ") {
    bool a = false, b = false, c = false;

    LambdaList list(
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

    const LambdaList l1([&x] { x += 1; }, [&x] { x += 2; }) ;
    const LambdaList l2([&x] { x += 3; });

    auto merged = concat(l1, l2);

    merged.forEach([](auto& f) { f(); });

    REQUIRE(x == 6);
}