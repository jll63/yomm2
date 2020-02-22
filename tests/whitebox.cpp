// Copyright (c) 2018-2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using std::is_same;
using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;

std::string to_string(const std::vector<rt_class*>& classes) {
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
    static const void* yOMM2_body(const Mammal& obj) {
        return &obj;
    }
};

struct get_this_carnivore {
    static const void* yOMM2_body(const Carnivore& obj) {
        return &obj;
    }
};

struct get_this_dog {
    static const void* yOMM2_body(const Dog& obj) {
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
         call(animal)) == &mammal);

    BOOST_TEST(
        (wrapper<voidp, get_this_carnivore,
         voidp(virtual_<const Animal&>), voidp(const Carnivore&)>::
         call(animal)) == &carnivore);

    BOOST_TEST(
        (wrapper<voidp, get_this_carnivore,
         voidp(virtual_<const Animal&>), voidp(const Carnivore&)>::
         call(animal)) == &carnivore);

    BOOST_TEST(
        (wrapper<voidp, get_this_dog,
         voidp(virtual_<const Animal&>), voidp(const Dog&)>::
         call(animal)) == &dog);
}


} // namespace casts

namespace rolex {

struct test_registry;

struct test_policy : default_policy {
    using registry = test_registry;
};

auto& registry = registry::get<test_registry>();
auto& dd = dispatch_data::instance<test_registry>::_;

struct Role {
    virtual ~Role() {}
};

struct Employee : Role {};
struct Manager : Employee {};
struct Founder : Role {};

struct Expense {
    virtual ~Expense() {}
};

struct Public : Expense {};
struct Bus : Public {};
struct Metro : Public {};
struct Taxi : Expense {};
struct Jet : Expense {};

YOMM2_CLASS_(test_registry, Role);
YOMM2_CLASS_(test_registry, Employee, Role);
YOMM2_CLASS_(test_registry, Manager, Employee);
YOMM2_CLASS_(test_registry, Founder, Role);
YOMM2_CLASS_(test_registry, Expense);
YOMM2_CLASS_(test_registry, Public, Expense);
YOMM2_CLASS_(test_registry, Bus, Public);
YOMM2_CLASS_(test_registry, Metro, Public);
YOMM2_CLASS_(test_registry, Taxi, Expense);
YOMM2_CLASS_(test_registry, Jet, Expense);

YOMM2_DECLARE(double, pay, (virtual_<const Employee&>), test_policy);
YOMM2_DECLARE(bool, approve, (virtual_<const Role&>, virtual_<const Expense&>, double), test_policy);

YOMM2_DEFINE(double, pay, (const Employee&)) {
    return 3000;
}

YOMM2_DEFINE(double, pay, (const Manager& exec)) {
    return next(exec) + 2000;
}

YOMM2_DEFINE(bool, approve, (const Role& r, const Expense& e, double amount)) {
    return false;
}

YOMM2_DEFINE(bool, approve, (const Employee& r, const Public& e, double amount)) {
    return true;
}

YOMM2_DEFINE(bool, approve, (const Manager& r, const Taxi& e, double amount)) {
    return true;
}

YOMM2_DEFINE(bool, approve, (const Founder& r, const Expense& e, double amount)) {
    return true;
}

const int num_classes = 10;

BOOST_AUTO_TEST_CASE(registration) {
    BOOST_TEST_REQUIRE(registry.classes.size() == num_classes);

    auto class_iter = registry.classes.begin();
    auto Role_class = *class_iter++;
    auto Employee_class = *class_iter++;
    auto Manager_class = *class_iter++;
    auto Founder_class = *class_iter++;
    auto Expense_class = *class_iter++;
    auto Public_class = *class_iter++;
    auto Bus_class = *class_iter++;
    auto Metro_class = *class_iter++;
    auto Taxi_class = *class_iter++;
    auto Jet_class = *class_iter++;

    BOOST_CHECK(class_iter == registry.classes.end());

    BOOST_TEST(Role_class->direct_bases.size() == 0);

    {
        std::vector<const class_info*> expected = { Role_class };
        BOOST_TEST_REQUIRE(Employee_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Employee_class };
        BOOST_TEST(Manager_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Role_class };
        BOOST_TEST(Founder_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Expense_class };
        BOOST_TEST_REQUIRE(Public_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Public_class };
        BOOST_TEST_REQUIRE(Bus_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Public_class };
        BOOST_TEST_REQUIRE(Metro_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Expense_class };
        BOOST_TEST(Taxi_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { Expense_class };
        BOOST_TEST_REQUIRE(Jet_class->direct_bases == expected);
    }

    auto pay_method = registry.methods[0];
    BOOST_TEST(pay_method->vp.size() == 1);
    BOOST_TEST(pay_method->specs.size() == 2);

    auto pay_method_iter = pay_method->specs.begin();
    auto pay_Employee = *pay_method_iter++;
    BOOST_TEST_REQUIRE(pay_Employee->vp.size() == 1);
    BOOST_TEST(pay_Employee->vp[0] == Employee_class);

    auto pay_Manager = pay_method_iter++;
    auto pay_Founder = pay_method_iter++;

    auto approve_method = registry.methods[1];
    BOOST_TEST(approve_method->vp.size() == 2);
    BOOST_TEST(approve_method->specs.size() == 4);

    auto approve_Employee_public = approve_method->specs[1];
    BOOST_TEST_REQUIRE(approve_Employee_public->vp.size() == 2);
    BOOST_TEST(approve_Employee_public->vp[0] == Employee_class);
    BOOST_TEST(approve_Employee_public->vp[1] == Public_class);
}

inline const word* mptr(const dispatch_data& t, const std::type_info* ti) {
    return t.hash_table[t.hash(ti)].pw;
}

BOOST_AUTO_TEST_CASE(runtime_test) {

    runtime rt(registry, dd);

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
    auto Role_class = &*class_iter++;
    auto Employee_class = &*class_iter++;
    auto Manager_class = &*class_iter++;
    auto Founder_class = &*class_iter++;
    auto Expense_class = &*class_iter++;
    auto Public_class = &*class_iter++;
    auto Bus_class = &*class_iter++;
    auto Metro_class = &*class_iter++;
    auto Taxi_class = &*class_iter++;
    auto Jet_class = &*class_iter++;

    BOOST_CHECK(class_iter == rt.classes.end());

    BOOST_TEST(Role_class->direct_bases.size() == 0);

    {
        std::vector<rt_class*> expected = { Role_class };
        BOOST_TEST(Employee_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Employee_class };
        BOOST_TEST(Manager_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Role_class };
        BOOST_TEST(Founder_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Expense_class };
        BOOST_TEST(Public_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Public_class };
        BOOST_TEST(Bus_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Public_class };
        BOOST_TEST(Metro_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Expense_class };
        BOOST_TEST(Taxi_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Expense_class };
        BOOST_TEST(Jet_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { Employee_class, Founder_class };
        BOOST_TEST(Role_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = { Manager_class };
        BOOST_TEST(Employee_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = {
            Public_class, Taxi_class, Jet_class
        };
        BOOST_TEST(Expense_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = { Bus_class, Metro_class };
        BOOST_TEST(Public_class->direct_derived == expected);
    }

    BOOST_TEST(Bus_class->direct_derived.size() == 0);
    BOOST_TEST(Metro_class->direct_derived.size() == 0);
    BOOST_TEST(Taxi_class->direct_derived.size() == 0);
    BOOST_TEST(Jet_class->direct_derived.size() == 0);

    rt.layer_classes();

    {
        std::vector<rt_class*> expected = {
          Role_class, Expense_class,
          Employee_class, Founder_class, Public_class, Taxi_class, Jet_class,
          Manager_class, Bus_class, Metro_class,
        };
        BOOST_TEST_INFO("result   = " + to_string(rt.layered_classes));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(rt.layered_classes == expected);
    }

    rt.calculate_conforming_classes();

    {
        std::unordered_set<rt_class*> expected = {
            Role_class, Employee_class, Founder_class, Manager_class
        };

        BOOST_TEST_INFO("result   = " + to_string(Role_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(Role_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = { Founder_class };

        BOOST_TEST_INFO("result   = " + to_string(Founder_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(Founder_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          Expense_class, Public_class, Taxi_class, Jet_class,
          Bus_class, Metro_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(Expense_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(Expense_class->conforming == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          Public_class, Bus_class, Metro_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(Public_class->conforming));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(Public_class->conforming == expected);
    }

    rt.augment_methods();

    BOOST_TEST_REQUIRE(rt.methods.size() == 2);
    BOOST_TEST(rt.methods[0].info == registry.methods[0]);
    rt_method& pay_method = rt.methods[0];
    BOOST_TEST(rt.methods[1].info == registry.methods[1]);
    rt_method& approve_method = rt.methods[1];

    {
        std::vector<rt_class*> expected = { Employee_class };
        BOOST_TEST_INFO("result   = " + to_string(pay_method.vp));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(pay_method.vp == expected);
    }

    {
        std::vector<rt_class*> expected = { Role_class, Expense_class };
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

    BOOST_TEST_REQUIRE(Role_class->vp.size() == 1);
    BOOST_TEST(Role_class->vp[0].method == &approve_method);
    BOOST_TEST(Role_class->vp[0].param == 0);

    BOOST_TEST_REQUIRE(Employee_class->vp.size() == 1);
    BOOST_TEST(Employee_class->vp[0].method == &pay_method);
    BOOST_TEST(Employee_class->vp[0].param == 0);

    BOOST_TEST_REQUIRE(Expense_class->vp.size() == 1);
    BOOST_TEST(Expense_class->vp[0].method == &approve_method);
    BOOST_TEST(Expense_class->vp[0].param == 1);

    rt.allocate_slots();

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == pay_method.slots);
    }

    {
        const std::vector<int> expected = { 0, 0 };
        BOOST_TEST(expected == approve_method.slots);
    }

    BOOST_TEST_REQUIRE(Role_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Employee_class->mtbl.size() == 2);
    BOOST_TEST_REQUIRE(Manager_class->mtbl.size() == 2);
    BOOST_TEST_REQUIRE(Founder_class->mtbl.size() == 1);

    BOOST_TEST_REQUIRE(Expense_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Public_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Bus_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Metro_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Taxi_class->mtbl.size() == 1);
    BOOST_TEST_REQUIRE(Jet_class->mtbl.size() == 1);

    auto pay_method_iter = pay_method.specs.begin();
    auto pay_Employee = pay_method_iter++;
    auto pay_Manager = pay_method_iter++;

    auto spec_iter = approve_method.specs.begin();
    auto approve_Role_Expense = spec_iter++;
    auto approve_Employee_public = spec_iter++;
    auto approve_Manager_Taxi = spec_iter++;
    auto approve_Founder_Expense = spec_iter++;

    {
        BOOST_TEST(
            runtime::is_more_specific(&*approve_Founder_Expense, &*approve_Role_Expense));
        BOOST_TEST(
            runtime::is_more_specific(&*approve_Manager_Taxi, &*approve_Role_Expense));
        BOOST_TEST(
            !runtime::is_more_specific(&*approve_Role_Expense, &*approve_Role_Expense));

        {
            std::vector<const rt_spec*> expected = { &*approve_Manager_Taxi };
            std::vector<const rt_spec*> specs = { &*approve_Role_Expense, &*approve_Manager_Taxi };
            BOOST_TEST(expected == runtime::best(specs));
        }
    }

    {
        BOOST_TEST(
            runtime::is_base(&*approve_Role_Expense, &*approve_Founder_Expense));
        BOOST_TEST(
            !runtime::is_base(&*approve_Role_Expense, &*approve_Role_Expense));
    }

    rt.build_dispatch_tables();

    {
        BOOST_TEST_REQUIRE(pay_method.dispatch_table.size() == 2);
        BOOST_TEST(pay_method.dispatch_table[0] == pay_Employee->info->pf);
        BOOST_TEST(pay_method.dispatch_table[1] == pay_Manager->info->pf);
    }

    {
        // expected dispatch table here:
        // https://www.codeproject.com/Articles/859492/Open-Multi-Methods-for-Cplusplus-Part-Inside-Yomm
        BOOST_TEST_REQUIRE(approve_method.dispatch_table.size() == 12);
        BOOST_TEST_REQUIRE(approve_method.strides.size() == 1);
        BOOST_TEST_REQUIRE(approve_method.strides[0] == 4);

        auto dp_iter = approve_method.dispatch_table.begin();
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Founder_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Employee_public->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Employee_public->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Founder_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Role_Expense->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Manager_Taxi->info->pf);
        BOOST_TEST(*dp_iter++ == approve_Founder_Expense->info->pf);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == Role_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1, 0 };
        BOOST_TEST(expected == Employee_class->mtbl);
    }

    {
        const std::vector<int> expected = { 2, 1 };
        BOOST_TEST(expected == Manager_class->mtbl);
    }

    {
        const std::vector<int> expected = { 3 };
        BOOST_TEST(expected == Founder_class->mtbl);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == Expense_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == Public_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == Bus_class->mtbl);
    }

    {
        const std::vector<int> expected = { 1 };
        BOOST_TEST(expected == Metro_class->mtbl);
    }

    {
        const std::vector<int> expected = { 2 };
        BOOST_TEST(expected == Taxi_class->mtbl);
    }

    {
        const std::vector<int> expected = { 0 };
        BOOST_TEST(expected == Jet_class->mtbl);
    }

    BOOST_TEST_REQUIRE(pay_Employee->info->next != nullptr);
    BOOST_TEST_REQUIRE(pay_Manager->info->next != nullptr);
    BOOST_TEST(*pay_Manager->info->next == pay_Employee->info->pf);

    rt.find_hash_function(rt.classes, rt.dd.hash, rt.metrics);
    rt.install_gv();

    {
        // pay
        BOOST_TEST_REQUIRE(dd.gv.size() ==
                           1       // ptr to control table
                           + rt.metrics.hash_table_size // mptr table
                           + rt.metrics.hash_table_size // control table
                           + 15    // approve: 3 slots and 12 cells for dispatch table
                           + 12);  // 3 mtbl of 2 cells for Roles + 6 mtbl of 1 cells for Expenses

        auto gv_iter = dd.hash_table + 2 * rt.metrics.hash_table_size;
        // no slots nor fun* for 1-method

        // approve
        BOOST_TEST(&*gv_iter == approve_method.info->slots_strides_p->pw);
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

        // Role
        BOOST_TEST(gv_iter++->i == 0); // approve/0
        // Employee
        BOOST_TEST(gv_iter++->i == 1); // approve/0
        BOOST_TEST(gv_iter++->i == 0); // pay
        // Manager
        BOOST_TEST(gv_iter++->i == 2); // approve/0
        BOOST_TEST(gv_iter++->i == 1); // pay
        // owner
        BOOST_TEST(gv_iter++->i == 3); // approve/0
        // Expense
        BOOST_TEST(gv_iter++->i == 0); // approve/1
        // Public
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // Bus
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // Metro
        BOOST_TEST(gv_iter++->i == 1); // approve/1
        // Taxi
        BOOST_TEST(gv_iter++->i == 2); // approve/1
        // Plane
        BOOST_TEST(gv_iter++->i == 0); // approve/1

        rt.optimize();

        // Role
        BOOST_TEST(opt_iter++->pw == 0 + approve_dispatch_table); // approve/0
        // Employee
        BOOST_TEST(opt_iter++->pw == 1 + approve_dispatch_table); // approve/0
        BOOST_TEST(opt_iter++->pf == pay_Employee->info->pf); // pay
        // Manager
        BOOST_TEST(opt_iter++->pw == 2 + approve_dispatch_table); // approve/0
        BOOST_TEST(opt_iter++->pf == pay_Manager->info->pf); // pay
        // owner
        BOOST_TEST(opt_iter++->pw == 3 + approve_dispatch_table); // approve/0
        // Expense
        BOOST_TEST(opt_iter++->i == 0); // approve/1
        // Public
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // Bus
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // Metro
        BOOST_TEST(opt_iter++->i == 1); // approve/1
        // Taxi
        BOOST_TEST(opt_iter++->i == 2); // approve/1
        // Plane
        BOOST_TEST(opt_iter++->i == 0); // approve/1

        BOOST_TEST(mptr(dd, &typeid(Role)) == Role_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Employee)) == Employee_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Manager)) == Manager_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Founder)) == Founder_class->mptr);

        BOOST_TEST(mptr(dd, &typeid(Expense)) == Expense_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Public)) == Public_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Bus)) == Bus_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Metro)) == Metro_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Taxi)) == Taxi_class->mptr);
        BOOST_TEST(mptr(dd, &typeid(Jet)) == Jet_class->mptr);
    }

    {
        const Role& role = Role();
        const Employee& employee = Employee();
        const Employee& manager = Manager();
        const Role& founder = Founder();

        const Expense& expense = Expense();
        const Expense& public_transport = Public();
        const Expense& bus = Bus();
        const Expense& metro = Metro();
        const Expense& taxi = Taxi();
        const Expense& jet = Jet();

        using pay_method = decltype(pay(discriminator(), Employee()));
        BOOST_TEST(pay_method::arity == 1);
        BOOST_TEST(pay_method::resolve(employee) == pay_Employee->info->pf);
        BOOST_TEST(&typeid(manager) == &typeid(Manager));
        BOOST_TEST(pay_method::resolve(manager) == pay_Manager->info->pf);

        using approve_method = decltype(approve(discriminator(), Role(), Expense(), 0.));
        BOOST_TEST(approve_method::arity == 2);

        BOOST_TEST(approve_method::resolve(role, expense, 0.) == approve_Role_Expense->info->pf);

        {
            std::vector<const Role*> Roles = {
                &role,
              &employee,
              &manager,
              &founder
            };

            std::vector<const Expense*> Expenses = {
                &expense,
              &public_transport,
              &bus,
              &metro,
              &taxi,
              &jet
            };

            int i = 0;

            for (auto r : Roles) {
                int j = 0;
                for (auto e : Expenses) {
                    BOOST_TEST_INFO("i = " << i << " j = " << j);
                    auto expected =
                        typeid(*r) == typeid(Founder) ? approve_Founder_Expense :
                        typeid(*r) == typeid(Manager) ?
                        (typeid(*e) == typeid(Taxi) ? approve_Manager_Taxi : dynamic_cast<const Public*>(e) ? approve_Employee_public : approve_Role_Expense) :
                        typeid(*r) == typeid(Employee) && dynamic_cast<const Public*>(e) ? approve_Employee_public :
                        approve_Role_Expense;
                    BOOST_TEST(approve_method::resolve(*r, *e, 0.) == expected->info->pf);
                    ++j;
                }
                ++i;
            }
        }

        BOOST_TEST(pay(employee) == 3000);
        BOOST_TEST(pay(manager) == 5000);
    }
}
}

namespace layer_mi {

struct test_registry;

struct test_policy : default_policy {
    using registry = test_registry;
};

auto& registry = registry::get<test_registry>();
auto& dd = dispatch_data::instance<test_registry>::_;

struct A0 {};

struct B0 {};

struct B1 : B0 {};

struct A1B2 : A0, B1 {};

YOMM2_CLASS_(test_registry, A0);
YOMM2_CLASS_(test_registry, B0);
YOMM2_CLASS_(test_registry, B1, B0);
YOMM2_CLASS_(test_registry, A1B2, A0, B1);

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

struct test_registry;

struct test_policy : default_policy {
    using registry = test_registry;
};

auto& registry = registry::get<test_registry>();
auto& dd = dispatch_data::instance<test_registry>::_;

// A   B
//  \ / \
//  AB   D
//  |    |
//  C    E

struct A {
    virtual ~A() {}
};

YOMM2_CLASS_(test_registry, A);

struct B {
    virtual ~B() {}
};

YOMM2_CLASS_(test_registry, B);

struct AB : A, B {};
YOMM2_CLASS_(test_registry, AB, A, B);

struct C : AB {};
YOMM2_CLASS_(test_registry, C, AB);

struct D : B {};
YOMM2_CLASS_(test_registry, D, B);

struct E : D {};
YOMM2_CLASS_(test_registry, E, D);

YOMM2_DECLARE(void, a, (virtual_<A&>), test_policy);
YOMM2_DECLARE(void, b, (virtual_<B&>), test_policy);
YOMM2_DECLARE(void, ab, (virtual_<A&>, virtual_<B&>), test_policy);
YOMM2_DECLARE(void, c, (virtual_<C&>), test_policy);
YOMM2_DECLARE(void, d, (virtual_<D&>), test_policy);

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
