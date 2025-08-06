#include <generic_host/meta/multibindings.hpp>
#include <generic_host/meta/bindings.hpp>

using namespace gh::boost_helpers;

struct IFoo {};
struct IBar {};
struct IBaz {};

struct FooImpl1: IFoo {};
struct FooImpl2: IFoo {};
struct FooImpl3: IFoo {};
struct BarImpl: IBar {};
struct BazImpl: IBaz {};

using B1 = Binding<IFoo, FooImpl1>;
using B2 = Binding<IFoo, FooImpl2>;
using B3 = Binding<IBar, BarImpl>;
using B4 = Binding<IBaz, BazImpl>;
using B5 = Binding<IFoo, FooImpl3>;

using BindingList = mp_list<B1, B2, B3, B4, B5>;

static_assert(is_binding_specialization<B1>::value);
static_assert(is_binding_specialization<B3>::value);
static_assert(!is_binding_specialization<int>::value);
static_assert(!is_binding_specialization<void>::value);

static_assert(std::is_same_v<InterfaceOf<B1>, IFoo>);
static_assert(std::is_same_v<ImplOf<B2>, FooImpl2>);

using KV = KeyValue<B3>;
static_assert(std::is_same_v<KV, mp_list<IBar, BarImpl>>);

using ExpectedKVList = mp_list<
    mp_list<IFoo, FooImpl1>,
    mp_list<IFoo, FooImpl2>,
    mp_list<IBar, BarImpl>,
    mp_list<IBaz, BazImpl>,
    mp_list<IFoo, FooImpl3>
>;

static_assert(std::is_same_v<
    KVList<BindingList>,
    ExpectedKVList
>);

using MaybeUniqueInterfaces = UniqueInterfaces<BindingList>;

static_assert(std::is_same_v<
    MaybeUniqueInterfaces,
    mp_list<IFoo, IBar, IBaz>
>);

using FooImplList = ImplListFor<IFoo, BindingList>;

static_assert(std::is_same_v<
    FooImplList,
    mp_list<FooImpl1, FooImpl2, FooImpl3>
>);

using BarImplList = ImplListFor<IBar, BindingList>;
static_assert(std::is_same_v<BarImplList, mp_list<BarImpl>>);

using FooGroup = GroupForInterface<BindingList, IFoo>::type;

static_assert(std::is_same_v<FooGroup::Interface, IFoo>);
static_assert(std::is_same_v<FooGroup::Implementations, mp_list<FooImpl1, FooImpl2, FooImpl3>>);

using Grouped = GroupedBindings<BindingList>;

// should be -> mp_list<MultiBindingGroup<IFoo, mp_list<FooImpl1, FooImpl2>>, MultiBindingGroup<IBar, mp_list<BarImpl>>, MultiBindingGroup<IBaz, mp_list<BazImpl>>>
using ExpectedGrouped = mp_list<
    MultiBindingGroup<IFoo, mp_list<FooImpl1, FooImpl2, FooImpl3>>,
    MultiBindingGroup<IBar, mp_list<BarImpl>>,
    MultiBindingGroup<IBaz, mp_list<BazImpl>>
>;

static_assert(std::is_same_v<Grouped, ExpectedGrouped>);

