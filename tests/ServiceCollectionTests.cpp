#define UNIT_TEST
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <generic_host/ServiceCollection.hpp>

using namespace gh;

struct IFoo {
    virtual int Value() const = 0;
    virtual ~IFoo() = default;
};

// ReSharper disable once CppClassCanBeFinal
struct Foo : IFoo {
    int _val = 42;
    int Value() const override { return _val; }
};

struct Bar {
    std::shared_ptr<IFoo> _foo;
    explicit Bar(std::shared_ptr<IFoo> foo): _foo(std::move(foo)) { }

    int GetValue() const { return _foo->Value(); }
};

TEST_CASE("AddSingleton with an instance should work", "[di]") {
    auto instance = std::make_shared<Foo>();
    const auto service = Services{}.AddSingletonAs<IFoo>(instance);

    const auto factory = service.getFactory<0>();
    std::shared_ptr<IFoo> result = factory();

    REQUIRE(result == instance); // OK: both are shared_ptr<Foo> pointing to same
    REQUIRE(result->Value() == instance->Value());

}