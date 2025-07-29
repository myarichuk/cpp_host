#define UNIT_TEST
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <generic_host/ServiceCollection.hpp>

using namespace gh;

struct IFoo {
    [[nodiscard]] virtual int Value() const = 0;
    virtual ~IFoo() = default;
};

// ReSharper disable once CppClassCanBeFinal
struct Foo : IFoo {
    int _val = 42;
    [[nodiscard]] int Value() const override { return _val; }
};

struct Bar {
    std::shared_ptr<IFoo> _foo;
    explicit Bar(std::shared_ptr<IFoo> foo): _foo(std::move(foo)) { }

    [[nodiscard]] int GetValue() const { return _foo->Value(); }
};

TEST_CASE("AddSingleton with an instance as interface should work", "[di]") {
    auto instance = std::make_shared<Foo>();
    const auto service = Services{}.AddSingletonAs<IFoo>(instance);

    const auto factory = service.GetFactory<0>();
    std::shared_ptr<IFoo> result = factory();

    REQUIRE(result == instance); // OK: both are shared_ptr<Foo> pointing to same
    REQUIRE(result->Value() == instance->Value());

}

TEST_CASE("AddSingleton with an instance should work", "[di]") {
    auto instance = std::make_shared<Foo>();
    const auto service = Services{}.AddSingleton(instance);

    const auto factory = service.GetFactory<0>();
    std::shared_ptr<Foo> result = factory();

    REQUIRE(result == instance); // OK: both are shared_ptr<Foo> pointing to same
    REQUIRE(result->Value() == instance->Value());

}

TEST_CASE("AddSingleton called twice should store two factories", "[di]") {
    auto instance = std::make_shared<Foo>();
    const auto service = Services{}
        .AddSingleton(instance)
        .AddSingletonAs<IFoo>(instance);

    const auto factory1 = service.GetFactory<0>();
    std::shared_ptr<Foo> result1 = factory1();

    REQUIRE(result1 == instance);
    REQUIRE(result1->Value() == instance->Value());

    const auto factory2 = service.GetFactory<1>();
    std::shared_ptr<IFoo> result2 = factory2();

    REQUIRE(result2 == instance);
    REQUIRE(result2->Value() == instance->Value());

    REQUIRE(result1 == result2);
}

TEST_CASE("AddSingleton for Foo and IFoo should resolve to the same instance", "[di]") {
    auto instance = std::make_shared<Foo>();
    auto services = Services{}
    .AddSingleton(instance)
    .AddSingletonAs<IFoo>(instance);

    auto injector = services.Build();

    auto resolvedFoo = injector.create<std::shared_ptr<Foo>>();
    auto resolvedIFoo = injector.create<std::shared_ptr<IFoo>>();

    REQUIRE(resolvedFoo != nullptr);
    REQUIRE(resolvedIFoo != nullptr);

    REQUIRE(resolvedFoo == instance);
    REQUIRE(resolvedIFoo == instance);

    REQUIRE(resolvedFoo == resolvedIFoo);

    REQUIRE(resolvedFoo->Value() == 42);
    REQUIRE(resolvedIFoo->Value() == 42);
}

TEST_CASE("AddTransient<IFoo, Foo> should allow resolving Bar with injected dependency", "[di]") {
    auto services = Services{}
    .AddTransient<IFoo, Foo>()
    .AddTransient<Bar>();

    auto injector = services.Build();

    auto bar1 = injector.create<std::shared_ptr<Bar>>();
    auto bar2 = injector.create<std::shared_ptr<Bar>>();

    REQUIRE(bar1 != nullptr);
    REQUIRE(bar2 != nullptr);
    REQUIRE(bar1 != bar2); // Transient => different Bar instances

    REQUIRE(bar1->GetValue() == 42);
    REQUIRE(bar2->GetValue() == 42);

    // under the hood, DI creates new Foo instances each time
    REQUIRE(bar1->_foo != bar2->_foo); // new IFoo (Foo) for each Bar
}

TEST_CASE("Singleton dependency with transient root should inject same instance into different root objects", "[di]") {
    auto singletonFoo = std::make_shared<Foo>();

    auto services = Services{}
    .AddSingletonAs<IFoo>(singletonFoo)
    .AddTransient<Bar>();

    auto injector = services.Build();

    auto bar1 = injector.create<std::shared_ptr<Bar>>();
    auto bar2 = injector.create<std::shared_ptr<Bar>>();

    REQUIRE(bar1 != nullptr);
    REQUIRE(bar2 != nullptr);
    REQUIRE(bar1 != bar2); // Because Bar is transient

    REQUIRE(bar1->_foo == singletonFoo);
    REQUIRE(bar2->_foo == singletonFoo);
    REQUIRE(bar1->_foo == bar2->_foo); // Same IFoo injected into both
}

TEST_CASE("Singleton dependency without instance but with transient root should inject same instance into different root objects", "[di]") {
    auto services = Services{}
    .AddSingleton<IFoo, Foo>()
    .AddTransient<Bar>();

    auto injector = services.Build();

    auto bar1 = injector.create<std::shared_ptr<Bar>>();
    auto bar2 = injector.create<std::shared_ptr<Bar>>();

    REQUIRE(bar1 != nullptr);
    REQUIRE(bar2 != nullptr);
    REQUIRE(bar1 != bar2); // because Bar is transient

    REQUIRE(bar1->_foo == bar2->_foo); // Same IFoo injected into both
}

TEST_CASE("Singleton dependency without instance and interface should always resolve the same instance", "[di]") {
    auto services = Services{}
    .AddSingleton<Foo>();

    auto injector = services.Build();
    auto foo1 = injector.create<std::shared_ptr<Foo>>();
    auto foo2 = injector.create<std::shared_ptr<Foo>>();

    REQUIRE(foo1 == foo2);
    REQUIRE(foo1->Value() == 42);
    REQUIRE(foo2->Value() == 42);
}

TEST_CASE("Both AddTransient overloads should register correctly", "[di]") {
    auto services = Services{}
    .AddTransient<IFoo, Foo>()
    .AddTransient<Bar>();

    auto injector = services.Build();

    auto foo = injector.create<std::shared_ptr<IFoo>>();
    auto bar = injector.create<std::shared_ptr<Bar>>();

    REQUIRE(foo != nullptr);
    REQUIRE(bar != nullptr);
    REQUIRE(bar->_foo != nullptr);

    REQUIRE(dynamic_cast<Foo*>(bar->_foo.get()) != nullptr);
    REQUIRE(dynamic_cast<Foo*>(foo.get()) != nullptr);

    REQUIRE(foo->Value() == 42);
    REQUIRE(bar->GetValue() == 42);
}
