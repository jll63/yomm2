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

std::string to_string(const std::vector<yomm2::rt_class*>& classes) {
    std::ostringstream os;
    os << "{ ";
    const char* sep = "";
    for (auto cls : classes) {
        os << sep << cls->info->name;
        sep = ", ";
    }
    os << " }";
    return os.str();
}

std::string to_string(const std::unordered_set<rt_class*>& classes) {
    std::vector<rt_class*> vec(classes.begin(), classes.end());
    std::sort(vec.begin(), vec.end()); // sort by address = good
    // std::sort(
    //     vec.begin(), vec.end(),
    //     [](rt_class* a, rt_class* b) {
    //         std::cerr << a->info->name << " <=> " << b->info->name << "\n";
    //         return strcmp(a->info->name, b->info->name) < 0;
    //     });
    return to_string(vec);
}

std::string to_string(const std::vector<const class_info*>& classes) {
    std::ostringstream os;
    os << "{ ";
    const char* sep = "";
    for (auto cls : classes) {
        os << sep << cls->name;
        sep = ", ";
    }
    os << " }";
    return os.str();
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

namespace matrices {

struct test;
auto& registry = yomm2::registry::get<test>();

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_CLASS_(test, matrix);
YOMM2_CLASS_(test, dense_matrix, matrix);
YOMM2_CLASS_(test, diagonal_matrix, matrix);

YOMM2_DECLARE_(test, void, times, virtual_<const matrix&>, virtual_<const matrix&>);
YOMM2_DECLARE_(test, void, times, double, virtual_<const matrix&>);
YOMM2_DECLARE_(test, void, times, virtual_<const matrix&>, double);

YOMM2_DEFINE(void, times, const matrix&, const matrix&) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix&, const diagonal_matrix&) {
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const diagonal_matrix& m) {
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const matrix& m) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix& m, double a) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const matrix& m, double a) {
} YOMM2_END;

BOOST_AUTO_TEST_CASE(compilation)
{
    update_methods(registry);
    const matrix& dense = dense_matrix();
    const matrix& diag = diagonal_matrix();
    times(dense, dense);
    times(2, dense);
    times(dense, 2);
    times(diag, dense);
    times(diag, diag);
}

BOOST_AUTO_TEST_CASE(registration)
{
    BOOST_TEST(registry.classes.size() == 3);
    BOOST_TEST(registry.classes[0]->direct_bases.size() == 0);
    BOOST_TEST_REQUIRE(registry.classes[1]->direct_bases.size() == 1);
    BOOST_TEST(registry.classes[1]->direct_bases[0] == registry.classes[0]);
    BOOST_TEST(registry.classes[2]->direct_bases.size() == 1);
    BOOST_TEST_REQUIRE(registry.classes[2]->direct_bases[0] == registry.classes[0]);

    BOOST_TEST(registry.methods.size() == 3);
    BOOST_TEST(registry.methods[0]->params.size() == 2);
    BOOST_TEST(registry.methods[0]->specs.size() == 2);

    BOOST_TEST(registry.methods[1]->specs.size() == 2);
    BOOST_TEST(registry.methods[1]->params.size() == 1);

    BOOST_TEST(registry.methods[2]->specs.size() == 2);
    BOOST_TEST(registry.methods[2]->params.size() == 1);
}

}

namespace rolex {

struct test;
auto& registry = yomm2::registry::get<test>();

struct role {
    virtual ~role() {}
};

struct employee : role {};
struct executive : employee {};
struct founder : role {};

struct expense {
    ~expense() {}
};

struct public_transport : expense {};
struct bus : public_transport {};
struct train : public_transport {};
struct cab : expense {};
struct jet : expense {};

YOMM2_CLASS_(test, role);
YOMM2_CLASS_(test, employee, role);
YOMM2_CLASS_(test, executive, employee);
YOMM2_CLASS_(test, founder, role);
YOMM2_CLASS_(test, expense);
YOMM2_CLASS_(test, public_transport, expense);
YOMM2_CLASS_(test, bus, public_transport);
YOMM2_CLASS_(test, train, public_transport);
YOMM2_CLASS_(test, cab, expense);
YOMM2_CLASS_(test, jet, expense);

YOMM2_DECLARE_(test, double, pay, virtual_<const role&>);
YOMM2_DECLARE_(test, bool, approve, virtual_<const role&>, virtual_<const expense&>, double);

YOMM2_DEFINE(double, pay, const employee&) {
    return 2000;
} YOMM2_END;

YOMM2_DEFINE(double, pay, const executive&) {
    return 5000;
} YOMM2_END;

YOMM2_DEFINE(double, pay, const founder&) {
    return 0;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const role& r, const expense& e, double amount) {
    return false;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const founder& r, const expense& e, double amount) {
    return true;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const employee& r, const public_transport& e, double amount) {
    return true;
} YOMM2_END;

YOMM2_DEFINE(bool, approve, const executive& r, const jet& e, double amount) {
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
    auto train_class = *class_iter++;
    auto cab_class = *class_iter++;
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
        BOOST_TEST_REQUIRE(train_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST(cab_class->direct_bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST_REQUIRE(jet_class->direct_bases == expected);
    }

    auto pay_method = registry.methods[0];
    BOOST_TEST(pay_method->params.size() == 1);
    BOOST_TEST(pay_method->specs.size() == 3);

    auto pay_employee = pay_method->specs[0];
    BOOST_TEST_REQUIRE(pay_employee->params.size() == 1);
    BOOST_TEST(pay_employee->params[0] == employee_class);

    auto approve_method = registry.methods[1];
    BOOST_TEST(approve_method->params.size() == 2);
    BOOST_TEST(approve_method->specs.size() == 4);

    auto approve_employee_public = approve_method->specs[2];
    BOOST_TEST_REQUIRE(approve_employee_public->params.size() == 2);
    BOOST_TEST(approve_employee_public->params[0] == employee_class);
    BOOST_TEST(approve_employee_public->params[1] == public_transport_class);
}

BOOST_AUTO_TEST_CASE(runtime_test) {
    using namespace yomm2;
    runtime rt(registry);

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
    auto train_class = &*class_iter++;
    auto cab_class = &*class_iter++;
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
        BOOST_TEST(train_class->direct_bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(cab_class->direct_bases == expected);
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
            public_transport_class, cab_class, jet_class
        };
        BOOST_TEST(expense_class->direct_derived == expected);
    }

    {
        std::vector<rt_class*> expected = { bus_class, train_class };
        BOOST_TEST(public_transport_class->direct_derived == expected);
    }

    BOOST_TEST(bus_class->direct_derived.size() == 0);
    BOOST_TEST(train_class->direct_derived.size() == 0);
    BOOST_TEST(cab_class->direct_derived.size() == 0);
    BOOST_TEST(jet_class->direct_derived.size() == 0);

    rt.layer_classes();

    {
        std::vector<rt_class*> expected = {
          role_class, expense_class,
          employee_class, founder_class, public_transport_class, cab_class, jet_class,
          executive_class, bus_class, train_class,
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

        BOOST_TEST_INFO("result   = " + to_string(role_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(role_class->confs == expected);
    }

    {
        std::unordered_set<rt_class*> expected = { founder_class };

        BOOST_TEST_INFO("result   = " + to_string(founder_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(founder_class->confs == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          expense_class, public_transport_class, cab_class, jet_class,
          bus_class, train_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(expense_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(expense_class->confs == expected);
    }

    {
        std::unordered_set<rt_class*> expected = {
          public_transport_class, bus_class, train_class,
        };

        BOOST_TEST_INFO("result   = " + to_string(public_transport_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(public_transport_class->confs == expected);
    }

    rt.augment_methods();

    BOOST_TEST_REQUIRE(rt.methods.size() == 2);
    BOOST_TEST(rt.methods[0].info == registry.methods[0]);
    rt_method& pay_method = rt.methods[0];
    BOOST_TEST(rt.methods[1].info == registry.methods[1]);
    rt_method& approve_method = rt.methods[1];

    {
        std::vector<rt_class*> expected = { role_class };
        BOOST_TEST_INFO("result   = " + to_string(pay_method.params));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(pay_method.params == expected);
    }

    {
        std::vector<rt_class*> expected = { role_class, expense_class };
        BOOST_TEST_INFO("result   = " << to_string(approve_method.params));
        BOOST_TEST_INFO("expected = " << to_string(expected));
        BOOST_TEST(approve_method.params == expected);
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
            BOOST_TEST_REQUIRE(rt_spec.params.size() == st_spec->params.size());
        }
    }

    BOOST_TEST_REQUIRE(role_class->method_params.size() == 2);
    BOOST_TEST(role_class->method_params[0].method == &pay_method);
    BOOST_TEST(role_class->method_params[0].param == 0);
    BOOST_TEST(role_class->method_params[1].method == &approve_method);
    BOOST_TEST(role_class->method_params[1].param == 0);

    BOOST_TEST_REQUIRE(expense_class->method_params.size() == 1);
    BOOST_TEST(expense_class->method_params[0].method == &approve_method);
    BOOST_TEST(expense_class->method_params[0].param == 1);

    rt.allocate_slots();
}
}

namespace multiple_inheritance {

struct test;
auto& registry = yomm2::registry::get<test>();

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
    using namespace yomm2;
    runtime rt(registry);
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
        std::cerr << m_ab->slots << "\n";
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
