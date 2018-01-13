#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#define BOOST_TEST_MODULE yomm2
//#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

using std::is_same;
namespace yomm2 = yorel::yomm2;
using yomm2::virtual_;
using namespace yomm2;
using namespace details;

std::string to_string(const std::vector<yomm2::rt_class*>& classes) {
#ifdef NDEBUG
    return "n/a";
#else
    std::ostringstream os;
    os << "{ ";
    const char* sep = "";
    for (auto cls : classes) {
        os << sep << cls->info->name;
        sep = ", ";
    }
    os << " }";
    return os.str();
#endif
}

std::string to_string(const std::unordered_set<rt_class*>& classes) {
#ifdef NDEBUG
    return "n/a";
#else
    std::vector<rt_class*> vec(classes.begin(), classes.end());
    std::sort(vec.begin(), vec.end()); // sort by address = good
    return to_string(vec);
#endif
}

std::string to_string(const std::vector<const class_info*>& classes) {
#ifdef NDEBUG
    return "n/a";
#else
    std::ostringstream os;
    os << "{ ";
    const char* sep = "";
    for (auto cls : classes) {
        os << sep << cls->name;
        sep = ", ";
    }
    os << " }";
    return os.str();
#endif
}

template<typename T>
std::ostream& operator <<(std::ostream& os, const std::vector<T>& vec) {
    os << "{ ";
    const char* sep = "";
    for (auto& val : vec) {
        os << sep << val;
        sep = ", ";
    }
    os << " }";
    return os;
}

namespace casts {

struct Animal {
    virtual ~Animal() {}
    int a{1};
};

struct Mammal : virtual Animal {
    int m{2};
};

struct Carnivore : virtual Animal {
    int c{3};
};

struct Dog : Mammal, Carnivore {
    int d{4};
};

struct get_this_mammal {
    static const void* body(const Mammal& obj) {
        return &obj;
    }
};

struct get_this_carnivore {
    static const void* body(const Carnivore& obj) {
        return &obj;
    }
};

struct get_this_dog {
    static const void* body(const Dog& obj) {
        return &obj;
    }
};

BOOST_AUTO_TEST_CASE(casts) {
    static_assert(
        is_same< select_cast_t<Animal, Mammal>, dynamic_cast_ >::value,
        "use dynamic_cast");
    static_assert(
        is_same< select_cast_t<Animal, Carnivore>, dynamic_cast_ >::value,
        "use dynamic_cast");
    static_assert(
        is_same< select_cast_t<Mammal, Dog>, static_cast_ >::value,
        "use dynamic_cast");
    static_assert(
        is_same< select_cast_t<Carnivore, Dog>, static_cast_ >::value,
        "use dynamic_cast");

    Dog dog;
    const Animal& animal = dog;
    const Mammal& mammal = dog;
    const Carnivore& carnivore = dog;
    BOOST_TEST(
        (&virtual_traits<virtual_<const Animal&>>::cast<const Mammal&>(
            animal, select_cast_t<Animal, Mammal>()).m) == &dog.m);
    BOOST_TEST(
        (&virtual_traits<virtual_<const Animal&>>::cast<const Carnivore&>(
            animal, select_cast_t<Animal, Mammal>()).c) == &dog.c);
    BOOST_TEST(
        (&virtual_traits<virtual_<const Animal&>>::cast<const Mammal&>(
            animal, select_cast_t<Animal, Mammal>()).m) == &dog.m);
    BOOST_TEST(
        (&virtual_traits<virtual_<const Animal&>>::cast<const Dog&>(
            animal, select_cast_t<Animal, Mammal>()).d) == &dog.d);
    BOOST_TEST(
        (&virtual_traits<virtual_<const Mammal&>>::cast<const Dog&>(
            mammal, select_cast_t<Mammal, Dog>()).d) == &dog.d);
    BOOST_TEST(
        (&virtual_traits<virtual_<const Carnivore&>>::cast<const Dog&>(
            carnivore, select_cast_t<Carnivore, Dog>()).c) == &dog.c);

    using voidp = const void*;
    using virtual_animal_t = virtual_base_t< virtual_<const Animal&> >;
    static_assert(std::is_same<virtual_animal_t, Animal>::value, "animal");
    using virtual_mammal_t = virtual_base_t<const Mammal&>;
    static_assert(std::is_same<virtual_mammal_t, Mammal>::value, "mammal");

    BOOST_TEST(
        (wrapper<voidp, get_this_mammal,
         voidp(virtual_<const Animal&>), voidp(const Mammal&)>::
         body(animal)) == &mammal);

    BOOST_TEST(
        (wrapper<voidp, get_this_carnivore,
         voidp(virtual_<const Animal&>), voidp(const Carnivore&)>::
         body(animal)) == &carnivore);

    BOOST_TEST(
        (wrapper<voidp, get_this_carnivore,
         voidp(virtual_<const Animal&>), voidp(const Carnivore&)>::
         body(animal)) == &carnivore);

    BOOST_TEST(
        (wrapper<voidp, get_this_dog,
         voidp(virtual_<const Animal&>), voidp(const Dog&)>::
         body(animal)) == &dog);
}


} // namespace casts

namespace rolex {

struct test;

auto& registry = yomm2::registry::get<test>();
auto& dd = yomm2::dispatch_data::instance<test>;

struct role {
    virtual ~role() {}
};

struct employee : role {};
struct executive : employee {};
struct founder : role {};

struct expense {
    virtual ~expense() {}
};

struct public_transport : expense {};
struct bus : public_transport {};
struct metro : public_transport {};
struct taxi : expense {};
struct jet : expense {};

YOMM2_CLASS_(test, role);
YOMM2_CLASS_(test, employee, role);
YOMM2_CLASS_(test, executive, employee);
YOMM2_CLASS_(test, founder, role);
YOMM2_CLASS_(test, expense);
YOMM2_CLASS_(test, public_transport, expense);
YOMM2_CLASS_(test, bus, public_transport);
YOMM2_CLASS_(test, metro, public_transport);
YOMM2_CLASS_(test, taxi, expense);
YOMM2_CLASS_(test, jet, expense);

YOMM2_DECLARE_(test, double, pay, virtual_<const employee&>);
YOMM2_DECLARE_(test, bool, approve, virtual_<const role&>, virtual_<const expense&>, double);

YOMM2_DEFINE(double, pay, const employee&) {
    return 3000;
} YOMM2_END;

YOMM2_DEFINE(double, pay, const executive& exec) {
    return next(exec) + 2000;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const role& r, const expense& e, double amount) {
    return false;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const employee& r, const public_transport& e, double amount) {
    return true;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const executive& r, const taxi& e, double amount) {
    return true;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const founder& r, const expense& e, double amount) {
    return true;
} YOMM2_END;

const int num_classes = 10;

BOOST_AUTO_TEST_CASE(registration) {
    using yomm2::class_info;

    BOOST_TEST_REQUIRE(registry.classes.size() == num_classes);

    auto class_iter = registry.classes.begin();
    auto role_class = *class_iter++;
    auto employee_class = *class_iter++;
    auto executive_class = *class_iter++;
    auto founder_class = *class_iter++;
    auto expense_class = *class_iter++;
    auto public_transport_class = *class_iter++;
    auto bus_class = *class_iter++;
    auto metro_class = *class_iter++;
    auto taxi_class = *class_iter++;
    auto jet_class = *class_iter++;

    BOOST_CHECK(class_iter == registry.classes.end());

    BOOST_TEST(role_class->direct_bases.size() == 0);

    {
        std::vector<const class_info*> expected = { role_class };
        BOOST_TEST_REQUIRE(employee_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { employee_class };
        BOOST_TEST(executive_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { role_class };
        BOOST_TEST(founder_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST_REQUIRE(public_transport_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { public_transport_class };
        BOOST_TEST_REQUIRE(bus_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { public_transport_class };
        BOOST_TEST_REQUIRE(metro_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST(taxi_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST_REQUIRE(jet_class->direct_bases == expected);
    }

    auto pay_method = registry.methods[0];
    BOOST_TEST(pay_method->vp.size() == 1);
    BOOST_TEST(pay_method->specs.size() == 2);

    auto pay_method_iter = pay_method->specs.begin();
    auto pay_employee = *pay_method_iter++;
    BOOST_TEST_REQUIRE(pay_employee->vp.size() == 1);
    BOOST_TEST(pay_employee->vp[0] == employee_class);

    auto pay_executive = pay_method_iter++;
    auto pay_founder = pay_method_iter++;

    auto approve_method = registry.methods[1];
    BOOST_TEST(approve_method->vp.size() == 2);
    BOOST_TEST(approve_method->specs.size() == 4);

    auto approve_employee_public = approve_method->specs[1];
    BOOST_TEST_REQUIRE(approve_employee_public->vp.size() == 2);
    BOOST_TEST(approve_employee_public->vp[0] == employee_class);
    BOOST_TEST(approve_employee_public->vp[1] == public_transport_class);
}

BOOST_AUTO_TEST_CASE(runtime_test) {

    runtime rt(registry, dd);

    details::log_on(&std::cerr);

    rt.augment_classes();

    BOOST_TEST_REQUIRE(rt.classes.size() == num_classes);

    {
        auto class_iter = registry.classes.begin();

        for (const auto& rt_class : rt.classes) {
            BOOST_TEST(rt_class.info == *class_iter);
            ++class_iter;
        }
    }

    auto class_iter = rt.classes.begin();
    auto role_class = &*class_iter++;
    auto employee_class = &*class_iter++;
    auto executive_class = &*class_iter++;
    auto founder_class = &*class_iter++;
    auto expense_class = &*class_iter++;
    auto public_transport_class = &*class_iter++;
    auto bus_class = &*class_iter++;
    auto metro_class = &*class_iter++;
    auto taxi_class = &*class_iter++;
    auto jet_class = &*class_iter++;

    BOOST_CHECK(class_iter == rt.classes.end());

    BOOST_TEST(role_class->direct_bases.size() == 0);

    {
        std::vector<rt_class*> expected = { role_class };
        BOOST_TEST(employee_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { employee_class };
        BOOST_TEST(executive_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { role_class };
        BOOST_TEST(founder_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(public_transport_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { public_transport_class };
        BOOST_TEST(bus_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { public_transport_class };
        BOOST_TEST(metro_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(taxi_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(jet_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { employee_class, founder_class };
        BOOST_TEST(role_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = { executive_class };
        BOOST_TEST(employee_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = {
            public_transport_class, taxi_class, jet_class
        };
        BOOST_TEST(expense_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = { bus_class, metro_class };
        BOOST_TEST(public_transport_class->direct_derived == expected);
    }

    BOOST_TEST(bus_class->direct_derived.size() == 0);
    BOOST_TEST(metro_class->direct_derived.size() == 0);
    BOOST_TEST(taxi_class->direct_derived.size() == 0);
    BOOST_TEST(jet_class->direct_derived.size() == 0);

    rt.layer_classes();

    {
        std::vector<rt_class*> expected = {
          role_class, expense_class,
          employee_class, founder_class, public_transport_class, taxi_class, jet_class,
          executive_class, bus_class, metro_class,
        };
        BOOST_TEST_INFO("result   = " + to_string(rt.layered_classes));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(rt.layered_classes == expected);
    }

    rt.calculate_conforming_classes();

    {
        std::unordered_set<rt_class*> expected = {
            role_class, employee_class, founder_class, executive_class
        };

        BOOST_TEST_INFO("result   = " + to_string(role_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(role_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = { founder_class };

        BOOST_TEST_INFO("result   = " + to_string(founder_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(founder_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          expense_class, public_transport_class, taxi_class, jet_class,
          bus_class, metro_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(expense_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(expense_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          public_transport_class, bus_class, metro_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(public_transport_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(public_transport_class->conforming == expected);
    }

    rt.augment_methods();

    BOOST_TEST_REQUIRE(rt.methods.size() == 2);
    BOOST_TEST(rt.methods[0].info == registry.methods[0]);
    rt_method& pay_method = rt.methods[0];
    BOOST_TEST(rt.methods[1].info == registry.methods[1]);
    rt_method& approve_method = rt.methods[1];

    {
        std::vector<rt_class*> expected = { employee_class };
        BOOST_TEST_INFO("result   = " + to_string(pay_method.vp));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(pay_method.vp == expected);
    }

    {
        std::vector<rt_class*> expected = { role_class, expense_class };
        BOOST_TEST_INFO("result   = " << to_string(approve_method.vp));
        BOOST_TEST_INFO("expected = " << to_string(expected));
        BOOST_TEST(approve_method.vp == expected);
    }

    for (int i = 0; i < rt.methods.size(); ++i) {
        BOOST_TEST_INFO("i = " << i);
        auto st_meth = registry.methods[i];
        auto& rt_meth = rt.methods[i];
        BOOST_TEST_REQUIRE(rt_meth.specs.size() == st_meth->specs.size());
        for (int j = 0; j < rt_meth.specs.size(); ++j) {
            BOOST_TEST_INFO("i = " << i);
            BOOST_TEST_INFO("j = " << j);
            auto st_spec = st_meth->specs[j];
            auto& rt_spec = rt_meth.specs[j];
            BOOST_TEST_REQUIRE(rt_spec.vp.size() == st_spec->vp.size());
        }
    }

    BOOST_TEST_REQUIRE(role_class->vp.size() == 1);
    BOOST_TEST(role_class->vp[0].method == &approve_method);
    BOOST_TEST(role_class->vp[0].param == 0);

    BOOST_TEST_REQUIRE(employee_class->vp.size() == 1);
    BOOST_TEST(employee_class->vp[0].method == &pay_method);
    BOOST_TEST(employee_class->vp[0].param == 0);

    BOOST_TEST_REQUIRE(expense_class->vp.size() == 1);
    BOOST_TEST(expense_class->vp[0].method == &approve_method);
    BOOST_TEST(expense_class->vp[0].param == 1);

    rt.allocate_slots();

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == pay_method.slots);
    }

    {
        const std::vector<int> expected = { 0, 0 };
        BOOST_TEST(expected == approve_method.slots);
    }

    BOOST_TEST_REQUIRE(role_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(employee_class->mtbl.size() == 2);
    BOOST_TEST_REQUIRE(executive_class->mtbl.size() == 2);
    BOOST_TEST_REQUIRE(founder_class->mtbl.size() == 1);

    BOOST_TEST_REQUIRE(expense_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(public_transport_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(bus_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(metro_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(taxi_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(jet_class->mtbl.size() == 1);

    auto pay_method_iter = pay_method.specs.begin();
    auto pay_employee = pay_method_iter++;
    auto pay_executive = pay_method_iter++;

    auto spec_iter = approve_method.specs.begin();
    auto approve_role_expense = spec_iter++;
    auto approve_employee_public = spec_iter++;
    auto approve_executive_taxi = spec_iter++;
    auto approve_founder_expense = spec_iter++;

    {
        BOOST_TEST(
            runtime::is_more_specific(&*approve_founder_expense, &*approve_role_expense));
        BOOST_TEST(
            runtime::is_more_specific(&*approve_executive_taxi, &*approve_role_expense));
        BOOST_TEST(
            !runtime::is_more_specific(&*approve_role_expense, &*approve_role_expense));

        {
            std::vector<const rt_spec*> expected = { &*approve_executive_taxi };
            std::vector<const rt_spec*> specs = { &*approve_role_expense, &*approve_executive_taxi };
            BOOST_TEST(expected == runtime::best(specs));
        }
    }

    rt.build_dispatch_tables();

    {
        BOOST_TEST_REQUIRE(pay_method.dispatch_table.size() == 2);
        BOOST_TEST(pay_method.dispatch_table[0] == pay_employee->info->pf);
        BOOST_TEST(pay_method.dispatch_table[1] == pay_executive->info->pf);
    }

    {
        // expected dispatch table here:
        // https://www.codeproject.com/Articles/859492/Open-Multi-Methods-for-Cplusplus-Part-Inside-Yomm
        BOOST_TEST_REQUIRE(approve_method.dispatch_table.size() == 12);
        BOOST_TEST_REQUIRE(approve_method.strides.size() == 1);
        BOOST_TEST_REQUIRE(approve_method.strides[0] == 4);

        auto dp_iter = approve_method.dispatch_table.begin();
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_founder_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_employee_public->info->pf);
        BOOST_TEST(*dp_iter++ == approve_employee_public->info->pf);
        BOOST_TEST(*dp_iter++ == approve_founder_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_role_expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_executive_taxi->info->pf);
        BOOST_TEST(*dp_iter++ == approve_founder_expense->info->pf);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == role_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1, 0 };
        BOOST_TEST(expected == employee_class->mtbl);
    }

    {
        const std::vector<int> expected = { 2, 1 };
        BOOST_TEST(expected == executive_class->mtbl);
    }

    {
        const std::vector<int> expected = { 3 };
        BOOST_TEST(expected == founder_class->mtbl);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == expense_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == public_transport_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == bus_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == metro_class->mtbl);
    }

    {
        const std::vector<int> expected = { 2 };
        BOOST_TEST(expected == taxi_class->mtbl);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == jet_class->mtbl);
    }

    BOOST_TEST_REQUIRE(pay_employee->info->next != nullptr);
    BOOST_TEST_REQUIRE(pay_executive->info->next != nullptr);
    BOOST_TEST(*pay_executive->info->next == pay_employee->info->pf);

    rt.find_hash_factor();
    rt.install_gv();

    {
        // pay
        BOOST_TEST_REQUIRE(dd.gv.size() ==
                           rt.metrics.hash_table_size
                           + 16
                           + 12);

        auto gv_iter = dd.gv.data() + rt.metrics.hash_table_size;
        BOOST_TEST(&*gv_iter == *pay_method.info->slots_strides_p);
        BOOST_TEST(gv_iter++->i == 1); // slot for pay
        // no fun* for 1-method

        // approve
        BOOST_TEST(&*gv_iter == *approve_method.info->slots_strides_p);
        BOOST_TEST(gv_iter++->i == 0); // slot for approve/0
        BOOST_TEST(gv_iter++->i == 0); // slot for approve/1
        BOOST_TEST(gv_iter++->i == 4); // stride for approve/1
        // 12 fun*
        auto approve_dispatch_table = gv_iter;
        BOOST_TEST(
            std::equal(
                approve_method.dispatch_table.begin(),
                approve_method.dispatch_table.end(),
                gv_iter,
                [](const void* pf, word w) { return w.pf == pf; }));
        gv_iter += approve_method.dispatch_table.size();

        auto opt_iter = gv_iter;

        // role
        BOOST_TEST(gv_iter++->i == 0); // approve/0
        // employee
        BOOST_TEST(gv_iter++->i == 1); // approve/0
        BOOST_TEST(gv_iter++->i == 0); // pay
        // executive
        BOOST_TEST(gv_iter++->i == 2); // approve/0
        BOOST_TEST(gv_iter++->i == 1); // pay
        // owner
        BOOST_TEST(gv_iter++->i == 3); // approve/0
        // expense
        BOOST_TEST(gv_iter++->i == 0); // approve/1
        // public_transport
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // bus
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // metro
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // taxi
        BOOST_TEST(gv_iter++->i == 2); // approve/1
        // plane
        BOOST_TEST(gv_iter++->i == 0); // approve/1

        rt.optimize();

        // role
        BOOST_TEST(opt_iter++->pw == 0 + approve_dispatch_table); // approve/0
        // employee
        BOOST_TEST(opt_iter++->pw == 1 + approve_dispatch_table); // approve/0
        BOOST_TEST(opt_iter++->pf == pay_employee->info->pf); // pay
        // executive
        BOOST_TEST(opt_iter++->pw == 2 + approve_dispatch_table); // approve/0
        BOOST_TEST(opt_iter++->pf == pay_executive->info->pf); // pay
        // owner
        BOOST_TEST(opt_iter++->pw == 3 + approve_dispatch_table); // approve/0
        // expense
        BOOST_TEST(opt_iter++->i == 0); // approve/1
        // public_transport
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // bus
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // metro
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // taxi
        BOOST_TEST(opt_iter++->i == 2); // approve/1
        // plane
        BOOST_TEST(opt_iter++->i == 0); // approve/1

        BOOST_TEST(mptr(dd, &typeid(role)) == role_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(employee)) == employee_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(executive)) == executive_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(founder)) == founder_class->mptr);

        BOOST_TEST(mptr(dd, &typeid(expense)) == expense_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(public_transport)) == public_transport_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(bus)) == bus_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(metro)) == metro_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(taxi)) == taxi_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(jet)) == jet_class->mptr);
    }

    {
        const role& a_role = role();
        const employee& a_employee = employee();
        const employee& a_executive = executive();
        const role& a_founder = founder();

        const expense& a_expense = expense();
        const expense& a_public_transport = public_transport();
        const expense& a_bus = bus();
        const expense& a_metro = metro();
        const expense& a_taxi = taxi();
        const expense& a_jet = jet();

        using pay_method = decltype(pay(discriminator(), employee()));
        BOOST_TEST(pay_method::arity == 1);
        BOOST_TEST(pay_method::resolve(a_employee) == pay_employee->info->pf);
        BOOST_TEST(&typeid(a_executive) == &typeid(executive));
        BOOST_TEST(pay_method::resolve(a_executive) == pay_executive->info->pf);

        using approve_method = decltype(approve(discriminator(), role(), expense(), 0.));
        BOOST_TEST(approve_method::arity == 2);

        BOOST_TEST(approve_method::resolve(a_role, a_expense, 0.) == approve_role_expense->info->pf);

        {
            std::vector<const role*> roles = {
                &a_role,
              &a_employee,
              &a_executive,
              &a_founder
            };

            std::vector<const expense*> expenses = {
                &a_expense,
              &a_public_transport,
              &a_bus,
              &a_metro,
              &a_taxi,
              &a_jet
            };

            int i = 0;

            for (auto r : roles) {
                int j = 0;
                for (auto e : expenses) {
                    BOOST_TEST_INFO("i = " << i << " j = " << j);
                    auto expected =
                        typeid(*r) == typeid(founder) ? approve_founder_expense :
                        typeid(*r) == typeid(executive) ?
                        (typeid(*e) == typeid(taxi) ? approve_executive_taxi : dynamic_cast<const public_transport*>(e) ? approve_employee_public : approve_role_expense) :
                        typeid(*r) == typeid(employee) && dynamic_cast<const public_transport*>(e) ? approve_employee_public :
                        approve_role_expense;
                    BOOST_TEST(approve_method::resolve(*r, *e, 0.) == expected->info->pf);
                    ++j;
                }
                ++i;
            }
        }

        BOOST_TEST(pay(a_employee) == 3000);
        BOOST_TEST(pay(a_executive) == 5000);
    }
}
}

namespace layer_mi {

struct test;
auto& registry = yomm2::registry::get<test>();
auto& dd = yomm2::dispatch_data::instance<test>;

struct A0 {};

struct B0 {};

struct B1 : B0 {};

struct A1B2 : A0, B1 {};

YOMM2_CLASS_(test, A0);
YOMM2_CLASS_(test, B0);
YOMM2_CLASS_(test, B1, B0);
YOMM2_CLASS_(test, A1B2, A0, B1);

BOOST_AUTO_TEST_CASE(test_layer_mi) {
    runtime rt(registry, dd);
    rt.augment_classes();
    rt.layer_classes();
    BOOST_TEST_REQUIRE(rt.layered_classes.size() == 4);
    auto layered_class_iter = rt.layered_classes.begin();
    auto class_iter = rt.classes.begin();
    auto a0_class = &*class_iter++;
    auto b0_class = &*class_iter++;
    auto b1_class = &*class_iter++;
    auto a1b2_class = &*class_iter++;
    BOOST_TEST(*layered_class_iter++ == a0_class);
    BOOST_TEST(a0_class->layer == 1);
    BOOST_TEST(*layered_class_iter++ == b0_class);
    BOOST_TEST(b0_class->layer == 1);
    BOOST_TEST(*layered_class_iter++ == b1_class);
    BOOST_TEST(b1_class->layer == 2);
    BOOST_TEST(*layered_class_iter++ == a1b2_class);
    BOOST_TEST(a1b2_class->layer == 3);
}

} // namespace layer_mi

namespace multiple_inheritance {

struct test;
auto& registry = yomm2::registry::get<test>();
auto& dd = yomm2::dispatch_data::instance<test>;

// A   B
//  \ / \
//  AB   D
//  |    |
//  C    E

struct A {};
YOMM2_CLASS_(test, A);

struct B {};
YOMM2_CLASS_(test, B);

struct AB : A, B {};
YOMM2_CLASS_(test, AB, A, B);

struct C : AB {};
YOMM2_CLASS_(test, C, AB);

struct D : B {};
YOMM2_CLASS_(test, D, B);

struct E : D {};
YOMM2_CLASS_(test, E, D);

YOMM2_DECLARE_(test, void, a, virtual_<A&>);
YOMM2_DECLARE_(test, void, b, virtual_<B&>);
YOMM2_DECLARE_(test, void, ab, virtual_<A&>, virtual_<B&>);
YOMM2_DECLARE_(test, void, c, virtual_<C&>);
YOMM2_DECLARE_(test, void, d, virtual_<D&>);

BOOST_AUTO_TEST_CASE(test_allocate_slots_mi) {
    runtime rt(registry, dd);
    rt.augment_classes();
    rt.layer_classes();
    rt.calculate_conforming_classes();
    rt.augment_methods();
    rt.allocate_slots();

    auto m_iter = rt.methods.begin();
    auto m_a = m_iter++;
    auto m_b = m_iter++;
    auto m_ab = m_iter++;
    auto m_c = m_iter++;
    auto m_d = m_iter++;

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == m_a->slots);
    }

    {
        const std::vector<int> expected = { 2 };
        BOOST_TEST(expected == m_b->slots);
    }

    {
        //BOOST_TEST_INFO("m_ab->slots" << m_ab->slots);
        const std::vector<int> expected = { 1, 3 };
        BOOST_TEST(expected == m_ab->slots);
    }

    {
        const std::vector<int> expected = { 4 };
        BOOST_TEST(expected == m_c->slots);
    }

    {
        const std::vector<int> expected = { 4 };
        BOOST_TEST(expected == m_d->slots);
    }
}

} // namespace multiple_inheritance
