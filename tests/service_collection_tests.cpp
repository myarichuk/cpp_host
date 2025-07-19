#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <generic_host/ServiceCollection.hpp>

using namespace generic_host;

struct IFoo {
    virtual int Value() const = 0;
    virtual ~IFoo() = default;
};

struct Foo : IFoo {
    int _val = 42;
    int Value() const override { return _val; }
};

struct Bar {
    std::shared_ptr<IFoo> _foo;
    explicit Bar(std::shared_ptr<IFoo> foo): _foo(std::move(foo)) { }

    int GetValue() const { return _foo->Value(); }
};

TEST_CASE("Should inject singleton into transient consumers", "[di]") {
    struct Foo1 : IFoo {
        int _val = 42;
        int Value() const override { return _val; }
    };

    struct Bar1 {
        std::shared_ptr<IFoo> _foo;
        explicit Bar1(std::shared_ptr<IFoo> foo): _foo(std::move(foo)) { }
        int GetValue() const { return _foo->Value(); }
    };

    auto services =
        Services{}.AddSingleton<IFoo, Foo1>()
                  .AddTransient<Bar1>();

    auto injector = services.Build();

    auto bar1 = injector.create<std::shared_ptr<Bar1>>();
    auto bar2 = injector.create<std::shared_ptr<Bar1>>();

    REQUIRE(bar1 != nullptr);
    REQUIRE(bar2 != nullptr);
    REQUIRE(bar1 != bar2); // transient => different Bar1s
    REQUIRE(bar1->GetValue() == 42);
    REQUIRE(bar2->GetValue() == 42);
    REQUIRE(bar1->_foo == bar2->_foo); // shared singleton instance
}

TEST_CASE("Should bind a specific shared_ptr instance as singleton", "[di]") {
    struct Foo2 : IFoo {
        int _val = 99;
        int Value() const override { return _val; }
    };

    struct Bar2 {
        std::shared_ptr<IFoo> _foo;
        explicit Bar2(std::shared_ptr<IFoo> foo): _foo(std::move(foo)) { }
        int GetValue() const { return _foo->Value(); }
    };

    auto customFoo = std::make_shared<Foo2>();
    auto services =
        Services{}.AddSingleton<IFoo, Foo2>(customFoo)
                  .AddTransient<Bar2>();

    auto injector = services.Build();

    auto bar = std::make_shared<Bar2>(injector.create<Bar2>());
    REQUIRE(bar->GetValue() == 99);
    REQUIRE(bar->_foo == customFoo);
}

struct Foo3 {
    int id;
    inline static int counter = 0;
    Foo3() {
        this->id = counter++;
    }
};

TEST_CASE("Should return new instances for AddTransient<T>", "[di]") {
    auto services = Services{}.AddTransient<Foo3>();
    auto injector = services.Build();

    auto foo1 = injector.create<Foo3>();
    auto foo2 = injector.create<Foo3>();

    REQUIRE(foo1.id != foo2.id);  // each instance is unique
}

struct Foo4 {
    int id;
    inline static int counter = 0;
    Foo4() {
        this->id = counter++;
    }
};

TEST_CASE("Should return same instance for AddSingleton<T>", "[di]") {
    auto services = Services{}.AddSingleton<Foo4>();
    auto injector = services.Build();

    auto foo1 = injector.create<std::shared_ptr<Foo4>>();
    auto foo2 = injector.create<std::shared_ptr<Foo4>>();

    REQUIRE(foo1 == foo2);          // pointer equality
    REQUIRE(foo1->id == foo2->id);  // value equality
}

TEST_CASE("Should inject same singleton into multiple transient consumers", "[di]") {
    struct Counter {
        int id = 123;
    };

    struct Consumer {
        std::shared_ptr<Counter> counter;
        explicit Consumer(std::shared_ptr<Counter> c) : counter(std::move(c)) {}
    };

    auto services = Services{}
        .AddSingleton<Counter>()
        .AddTransient<Consumer>();

    auto injector = services.Build();

    auto c1 = injector.create<Consumer>();
    auto c2 = injector.create<Consumer>();

    REQUIRE(c1.counter == c2.counter);  // both consumers share the same singleton
}

TEST_CASE("Should resolve singleton service as shared_ptr<T>", "[di]") {
    struct Foo5 {
        int val;
        // should be explicit or boost::di won't initialize
        Foo5(): val(999) {
        }
    };

    auto services = Services{}
        .AddSingleton<Foo5>();

    auto injector = services.Build();

    auto f = injector.create<std::shared_ptr<Foo5>>();
    REQUIRE(f->val == 999);
}
