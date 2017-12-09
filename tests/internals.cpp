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

BOOST_AUTO_TEST_CASE(registration)
{
    const int num_classes = 10;

    {
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

        BOOST_TEST_REQUIRE(employee_class->bases.size() == 1);
        BOOST_TEST(employee_class->bases[0] == role_class);

        BOOST_TEST_REQUIRE(executive_class->bases.size() == 1);
        BOOST_TEST(executive_class->bases[0] == employee_class);

        BOOST_TEST_REQUIRE(founder_class->bases.size() == 1);
        BOOST_TEST(founder_class->bases[0] == role_class);

        BOOST_TEST(expense_class->bases.size() == 0);

        BOOST_TEST_REQUIRE(public_transport_class->bases.size() == 1);
        BOOST_TEST(public_transport_class->bases[0] == expense_class);

        BOOST_TEST_REQUIRE(bus_class->bases.size() == 1);
        BOOST_TEST(bus_class->bases[0] == public_transport_class);

        BOOST_TEST_REQUIRE(train_class->bases.size() == 1);
        BOOST_TEST(train_class->bases[0] == public_transport_class);

        BOOST_TEST_REQUIRE(cab_class->bases.size() == 1);
        BOOST_TEST(cab_class->bases[0] == expense_class);

        BOOST_TEST_REQUIRE(jet_class->bases.size() == 1);
        BOOST_TEST(jet_class->bases[0] == expense_class);

        BOOST_TEST_REQUIRE(registry.methods.size() == 2);

        auto pay_method = registry.methods[0];
        BOOST_TEST(pay_method->vargs.size() == 1);
        BOOST_TEST(pay_method->specs.size() == 3);

        auto approve_method = registry.methods[1];
        BOOST_TEST(approve_method->vargs.size() == 2);
        BOOST_TEST(approve_method->specs.size() == 4);
    }

    {
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

        BOOST_TEST_REQUIRE(employee_class->direct_bases.size() == 1);
        BOOST_TEST(employee_class->direct_bases[0] == role_class);

        BOOST_TEST_REQUIRE(executive_class->direct_bases.size() == 1);
        BOOST_TEST(executive_class->direct_bases[0] == employee_class);

        BOOST_TEST_REQUIRE(founder_class->direct_bases.size() == 1);
        BOOST_TEST(founder_class->direct_bases[0] == role_class);

        BOOST_TEST_REQUIRE(public_transport_class->direct_bases.size() == 1);
        BOOST_TEST(public_transport_class->direct_bases[0] == expense_class);

        BOOST_TEST_REQUIRE(bus_class->direct_bases.size() == 1);
        BOOST_TEST(bus_class->direct_bases[0] == public_transport_class);

        BOOST_TEST_REQUIRE(train_class->direct_bases.size() == 1);
        BOOST_TEST(train_class->direct_bases[0] == public_transport_class);

        BOOST_TEST_REQUIRE(cab_class->direct_bases.size() == 1);
        BOOST_TEST(cab_class->direct_bases[0] == expense_class);

        BOOST_TEST_REQUIRE(jet_class->direct_bases.size() == 1);
        BOOST_TEST(jet_class->direct_bases[0] == expense_class);

        BOOST_TEST_REQUIRE(role_class->direct_specs.size() == 2);
        BOOST_TEST(role_class->direct_specs[0] == employee_class);
        BOOST_TEST(role_class->direct_specs[1] == founder_class);

        BOOST_TEST_REQUIRE(employee_class->direct_specs.size() == 1);
        BOOST_TEST(employee_class->direct_specs[0] == executive_class);

        BOOST_TEST(executive_class->direct_specs.size() == 0);
        BOOST_TEST(founder_class->direct_specs.size() == 0);

        BOOST_TEST_REQUIRE(expense_class->direct_specs.size() == 3);
        BOOST_TEST(expense_class->direct_specs[0] == public_transport_class);
        BOOST_TEST(expense_class->direct_specs[1] == cab_class);
        BOOST_TEST(expense_class->direct_specs[2] == jet_class);

        BOOST_TEST_REQUIRE(public_transport_class->direct_specs.size() == 2);
        BOOST_TEST(public_transport_class->direct_specs[0] == bus_class);
        BOOST_TEST(public_transport_class->direct_specs[1] == train_class);

        BOOST_TEST_REQUIRE(bus_class->direct_specs.size() == 0);
        BOOST_TEST_REQUIRE(train_class->direct_specs.size() == 0);
        BOOST_TEST_REQUIRE(cab_class->direct_specs.size() == 0);
        BOOST_TEST_REQUIRE(jet_class->direct_specs.size() == 0);
    }
}

}
