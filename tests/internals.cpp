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
        os << sep << cls->info->description;
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
    //         std::cerr << a->info->description << " <=> " << b->info->description << "\n";
    //         return strcmp(a->info->description, b->info->description) < 0;
    //     });
    return to_string(vec);
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
    BOOST_TEST(registry.classes[0]->bases.size() == 0);
    BOOST_TEST_REQUIRE(registry.classes[1]->bases.size() == 1);
    BOOST_TEST(registry.classes[1]->bases[0] == registry.classes[0]);
    BOOST_TEST(registry.classes[2]->bases.size() == 1);
    BOOST_TEST_REQUIRE(registry.classes[2]->bases[0] == registry.classes[0]);

    BOOST_TEST(registry.methods.size() == 3);
    BOOST_TEST(registry.methods[0]->vargs.size() == 2);
    BOOST_TEST(registry.methods[0]->specs.size() == 2);

    BOOST_TEST(registry.methods[1]->specs.size() == 2);
    BOOST_TEST(registry.methods[1]->vargs.size() == 1);

    BOOST_TEST(registry.methods[2]->specs.size() == 2);
    BOOST_TEST(registry.methods[2]->vargs.size() == 1);
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

    BOOST_TEST(role_class->bases.size() == 0);

    {
        std::vector<const class_info*> expected = { role_class };
        BOOST_TEST_REQUIRE(employee_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { employee_class };
        BOOST_TEST(executive_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { role_class };
        BOOST_TEST(founder_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST_REQUIRE(public_transport_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { public_transport_class };
        BOOST_TEST_REQUIRE(bus_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { public_transport_class };
        BOOST_TEST_REQUIRE(train_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST(cab_class->bases == expected);
    }

    {
        std::vector<const class_info*> expected = { expense_class };
        BOOST_TEST_REQUIRE(jet_class->bases == expected);
    }

    auto pay_method = registry.methods[0];
    BOOST_TEST(pay_method->vargs.size() == 1);
    BOOST_TEST(pay_method->specs.size() == 3);

    auto pay_employee = pay_method->specs[0];
    BOOST_TEST_REQUIRE(pay_employee->vargs.size() == 1);
    BOOST_TEST(pay_employee->vargs[0] == employee_class);

    auto approve_method = registry.methods[1];
    BOOST_TEST(approve_method->vargs.size() == 2);
    BOOST_TEST(approve_method->specs.size() == 4);

    auto approve_employee_public = approve_method->specs[2];
    BOOST_TEST_REQUIRE(approve_employee_public->vargs.size() == 2);
    BOOST_TEST(approve_employee_public->vargs[0] == employee_class);
    BOOST_TEST(approve_employee_public->vargs[1] == public_transport_class);
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

    BOOST_TEST(role_class->bases.size() == 0);

    {
        std::vector<rt_class*> expected = { role_class };
        BOOST_TEST(employee_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { employee_class };
        BOOST_TEST(executive_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { role_class };
        BOOST_TEST(founder_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(public_transport_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { public_transport_class };
        BOOST_TEST(bus_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { public_transport_class };
        BOOST_TEST(train_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(cab_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { expense_class };
        BOOST_TEST(jet_class->bases == expected);
    }

    {
        std::vector<rt_class*> expected = { employee_class, founder_class };
        BOOST_TEST(role_class->specs == expected);
    }

    {
        std::vector<rt_class*> expected = { executive_class };
        BOOST_TEST(employee_class->specs == expected);
    }

    {
        std::vector<rt_class*> expected = {
            public_transport_class, cab_class, jet_class
        };
        BOOST_TEST(expense_class->specs == expected);
    }

    {
        std::vector<rt_class*> expected = { bus_class, train_class };
        BOOST_TEST(public_transport_class->specs == expected);
    }

    BOOST_TEST(bus_class->specs.size() == 0);
    BOOST_TEST(train_class->specs.size() == 0);
    BOOST_TEST(cab_class->specs.size() == 0);
    BOOST_TEST(jet_class->specs.size() == 0);

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
        BOOST_TEST_INFO("result   = " + to_string(public_transport_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(pay_method.vargs == expected);
    }

    {
        std::vector<rt_class*> expected = { role_class, expense_class };
        BOOST_TEST_INFO("result   = " + to_string(public_transport_class->confs));
        BOOST_TEST_INFO("expected = " + to_string(expected));
        BOOST_TEST(approve_method.vargs == expected);
    }

    BOOST_TEST_REQUIRE(rt.methods.size() == registry.methods.size());

    // for (int i = 0; i < rt.methods.size(); ++i) {
    //     BOOST_TEST_INFO("i = " << i);
    //     auto st_meth = registry.methods[i];
    //     auto& rt_meth = rt.methods[i];
    //     BOOST_TEST_REQUIRE(rt_meth.specs.size() == st_meth->specs.size());
    //     for (int j = 0; j < rt_meth.specs.size(); ++j) {
    //         auto st_spec = st_meth->specs[j];
    //         auto& rt_spec = rt_meth.specs[j];
    //         BOOST_TEST(rt_spec.args.size() > 0);
    //         BOOST_TEST_REQUIRE(rt_spec.args.size() == st_spec->args.size());
    //     }

    // }
}


}
