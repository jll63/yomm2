// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;

namespace states {

using test_policy = test_policy_<__COUNTER__>;
using std::string;

struct Animal {
    Animal(const Animal&) = delete;
    Animal() : name("wrong") {
    }

    virtual ~Animal() {
    }

    std::string name;
};

struct Dog : Animal {
    Dog(string n) {
        name = n;
    }
};

struct Cat : virtual Animal {
    Cat(string n) {
        name = n;
    }
};

YOMM2_CLASSES(Animal, Dog, Cat, test_policy);

YOMM2_METHOD(name, (virtual_<const Animal&>), string, test_policy);

YOMM2_OVERRIDE(name, (const Cat& cat), string) {
    return "cat " + cat.name;
}

YOMM2_OVERRIDE(name, (const Dog& dog), string) {
    return "dog " + dog.name;
}

BOOST_AUTO_TEST_CASE(initializing) {
    initialize<test_policy>();
    const Animal& dog = Dog("spot");
    BOOST_TEST("dog spot" == name(dog));
    const Animal& cat = Cat("felix");
    BOOST_TEST("cat felix" == name(cat));
}

} // namespace states

namespace matrices {

struct matrix {
    virtual ~matrix() {
    }
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

enum Subtype {
    MATRIX,
    DIAGONAL,
    SCALAR_MATRIX,
    SCALAR_DIAGONAL,
    MATRIX_SCALAR,
    DIAGONAL_SCALAR,
    MATRIX_MATRIX,
    DIAGONAL_DIAGONAL
};

YOMM2_CLASSES(matrix, dense_matrix, diagonal_matrix);

YOMM2_METHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), Subtype);
YOMM2_METHOD(times, (double, virtual_<const matrix&>), Subtype);
YOMM2_METHOD(times, (virtual_<const matrix&>, double), Subtype);

YOMM2_OVERRIDE(times, (const matrix&, const matrix&), Subtype) {
    return MATRIX_MATRIX;
}

YOMM2_OVERRIDE(times, (const diagonal_matrix&, const diagonal_matrix&), Subtype) {
    return DIAGONAL_DIAGONAL;
}

YOMM2_OVERRIDE(times, (double a, const matrix& m), Subtype) {
    return SCALAR_MATRIX;
}

YOMM2_OVERRIDE(times, (double a, const diagonal_matrix& m), Subtype) {
    return SCALAR_DIAGONAL;
}

YOMM2_OVERRIDE(times, (const diagonal_matrix& m, double a), Subtype) {
    return DIAGONAL_SCALAR;
}

YOMM2_OVERRIDE(times, (const matrix& m, double a), Subtype) {
    return MATRIX_SCALAR;
}

YOMM2_METHOD(times, (virtual_<matrix&&>, virtual_<matrix&&>), int);
YOMM2_METHOD(times, (double, virtual_<matrix&&>), int);
YOMM2_METHOD(times, (virtual_<matrix&&>, double), int);

YOMM2_OVERRIDE(times, (matrix&&, matrix&&), int) {
    return -MATRIX_MATRIX;
}

YOMM2_OVERRIDE(times, (diagonal_matrix&&, diagonal_matrix&&), int) {
    return -DIAGONAL_DIAGONAL;
}

YOMM2_OVERRIDE(times, (double a, matrix&& m), int) {
    return -SCALAR_MATRIX;
}

YOMM2_OVERRIDE(times, (double a, diagonal_matrix&& m), int) {
    return -SCALAR_DIAGONAL;
}

YOMM2_OVERRIDE(times, (diagonal_matrix && m, double a), int) {
    return -DIAGONAL_SCALAR;
}

YOMM2_OVERRIDE(times, (matrix && m, double a), int) {
    return -MATRIX_SCALAR;
}

YOMM2_METHOD(zero_ref, (virtual_<matrix&>), Subtype);

YOMM2_OVERRIDE(zero_ref, (dense_matrix & m), Subtype) {
    return MATRIX;
}

YOMM2_OVERRIDE(zero_ref, (diagonal_matrix & m), Subtype) {
    return DIAGONAL;
}

BOOST_AUTO_TEST_CASE(simple) {
    auto report = initialize();

    {
        const matrix& dense = dense_matrix();
        const matrix& diag = diagonal_matrix();
        BOOST_TEST(times(dense, dense) == MATRIX_MATRIX);
        BOOST_TEST(times(diag, diag) == DIAGONAL_DIAGONAL);
        BOOST_TEST(times(diag, dense) == MATRIX_MATRIX);
        BOOST_TEST(times(2, dense) == SCALAR_MATRIX);
        BOOST_TEST(times(dense, 2) == MATRIX_SCALAR);
        BOOST_TEST(times(diag, 2) == DIAGONAL_SCALAR);
    }

    {
        BOOST_TEST(times(dense_matrix(), dense_matrix()) == -MATRIX_MATRIX);
        BOOST_TEST(
            times(diagonal_matrix(), diagonal_matrix()) == -DIAGONAL_DIAGONAL);
        BOOST_TEST(times(diagonal_matrix(), dense_matrix()) == -MATRIX_MATRIX);
        BOOST_TEST(times(2, dense_matrix()) == -SCALAR_MATRIX);
        BOOST_TEST(times(dense_matrix(), 2) == -MATRIX_SCALAR);
        BOOST_TEST(times(diagonal_matrix(), 2) == -DIAGONAL_SCALAR);
    }

    {
        dense_matrix dense;
        BOOST_TEST(zero_ref(dense) == MATRIX);
        diagonal_matrix diagonal;
        BOOST_TEST(zero_ref(diagonal) == DIAGONAL);
    }
}

} // namespace matrices

namespace test_next_fn {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};
struct Bulldog : Dog {};

register_classes(Animal, Dog, Bulldog);

struct YOMM2_METHOD_NAME(kick);
using kick = method<YOMM2_METHOD_NAME(kick)(virtual_<Animal&>), std::string>;

std::string kick_dog(Dog& dog) {
    return "bark";
}

YOMM2_REGISTER(kick::override_fn<kick_dog>);

std::string kick_bulldog(Bulldog& dog) {
    return kick::next<kick_bulldog>(dog) + " and bite back";
}

YOMM2_REGISTER(kick::override_fn<kick_bulldog>);

BOOST_AUTO_TEST_CASE(test_next_fn) {
    initialize();

    std::unique_ptr<Animal> snoopy = std::make_unique<Dog>();
    BOOST_TEST(kick::fn(*snoopy) == "bark");

    std::unique_ptr<Animal> hector = std::make_unique<Bulldog>();
    BOOST_TEST(kick::fn(*hector) == "bark and bite back");
}

} // namespace test_next_fn

namespace errors {

struct matrix {
    virtual ~matrix() {
    }
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_CLASSES(matrix, dense_matrix, diagonal_matrix, matrix);

YOMM2_METHOD(times, (virtual_<const matrix&>, virtual_<const matrix&>), void);

YOMM2_OVERRIDE(times, (const diagonal_matrix&, const matrix&), void) {
}

YOMM2_OVERRIDE(times, (const matrix&, const diagonal_matrix&), void) {
}

void test_handler(const default_policy::error_variant& error_v) {
    if (auto error = std::get_if<resolution_error>(&error_v)) {
        throw *error;
    }

    if (auto error = std::get_if<unknown_class_error>(&error_v)) {
        throw *error;
    }

    if (auto error = std::get_if<hash_search_error>(&error_v)) {
        throw *error;
    }

    throw int();
}

} // namespace errors

namespace initialize_error_handling {

using test_policy = test_policy_<__COUNTER__>;

struct base {
    virtual ~base() {
    }
};

YOMM2_METHOD(foo, (virtual_<base&>), void, test_policy);

BOOST_AUTO_TEST_CASE(test_initialize_error_handling) {
    auto prev_handler = test_policy::set_error_handler(errors::test_handler);

    try {
        initialize<test_policy>();
    } catch (const unknown_class_error& error) {
        test_policy::set_error_handler(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(base)));
        return;
    } catch (...) {
        test_policy::set_error_handler(prev_handler);
        BOOST_FAIL("unexpected exception");
    }
    BOOST_FAIL("did not throw");
}
} // namespace initialize_error_handling

namespace across_namespaces {

namespace animals {

class Animal {
  public:
    virtual ~Animal() {
    }
};

YOMM2_METHOD(kick, (virtual_<const Animal&>), std::string);

} // namespace animals

namespace more_animals {

class Dog : public animals::Animal {};

YOMM2_CLASSES(Dog, animals::Animal);

YOMM2_OVERRIDE(kick, (const Dog& dog), std::string) {
    return "bark";
}

} // namespace more_animals

BOOST_AUTO_TEST_CASE(across_namespaces) {
    const animals::Animal& animal = more_animals::Dog();
    BOOST_TEST("bark" == kick(animal));
}

} // namespace across_namespaces

namespace report {

using test_policy = test_policy_<__COUNTER__>;

struct Animal {
    virtual void foo() = 0;
};

struct Dog : Animal {
    void foo() override {
    }
};

struct Cat : Animal {
    void foo() override {
    }
};

struct kick_;
struct pet_;
struct meet_;

template<class... Class>
void fn(Class&...) {
}

YOMM2_CLASSES(Animal, Dog, Cat, test_policy);

BOOST_AUTO_TEST_CASE(update_report) {
    using kick = method<kick_(virtual_<Animal&>), void, test_policy>;
    using pet = method<pet_(virtual_<Animal&>), void, test_policy>;
    using meet =
        method<meet_(virtual_<Animal&>, virtual_<Animal&>), void, test_policy>;

    auto report = initialize<test_policy>().report;
    BOOST_TEST(report.not_implemented == 3);
    BOOST_TEST(report.ambiguous == 0);
    // 'meet' dispatch table is one cell, containing 'not_implemented'
    BOOST_TEST(report.cells == 1);

    YOMM2_REGISTER(kick::override_fn<fn<Animal>>);
    report = initialize<test_policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    YOMM2_REGISTER(pet::override_fn<fn<Cat>>);
    YOMM2_REGISTER(pet::override_fn<fn<Dog>>);
    report = initialize<test_policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    // create ambiguity
    YOMM2_REGISTER(meet::override_fn<fn<Animal, Cat>>);
    YOMM2_REGISTER(meet::override_fn<fn<Dog, Animal>>);
    report = initialize<test_policy>().report;
    BOOST_TEST(report.cells == 4);
    BOOST_TEST(report.ambiguous == 1);

    YOMM2_REGISTER(meet::override_fn<fn<Cat, Cat>>);
    report = initialize<test_policy>().report;
    BOOST_TEST(report.cells == 6);
    BOOST_TEST(report.ambiguous == 1);

    // shadow ambiguity
    YOMM2_REGISTER(meet::override_fn<fn<Dog, Dog>>);
    YOMM2_REGISTER(meet::override_fn<fn<Dog, Cat>>);
    YOMM2_REGISTER(meet::override_fn<fn<Cat, Dog>>);
    report = initialize<test_policy>().report;
    BOOST_TEST(report.cells == 9);
    BOOST_TEST(report.ambiguous == 0);
}

} // namespace report

namespace test_comma_in_return_type {

using test_policy = test_policy_<__COUNTER__>;

struct Test {
    virtual ~Test() {};
};

YOMM2_CLASSES(Test, test_policy);

YOMM2_METHOD(foo, (virtual_<Test&>), std::pair<int, int>, test_policy);

YOMM2_OVERRIDE(foo, (Test&), std::pair<int, int>) {
    return {1, 2};
}

BOOST_AUTO_TEST_CASE(comma_in_return_type) {
    initialize<test_policy>();

    Test test;

    BOOST_TEST((foo(test) == std::pair(1, 2)));
}

} // namespace test_comma_in_return_type
