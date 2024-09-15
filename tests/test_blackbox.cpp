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

YOMM2_DECLARE(string, name, (virtual_<const Animal&>), test_policy);

YOMM2_DEFINE(string, name, (const Dog& dog)) {
    return "dog " + dog.name;
}

YOMM2_DEFINE(string, name, (const Cat& cat)) {
    return "cat " + cat.name;
}

BOOST_AUTO_TEST_CASE(initializing) {
    update<test_policy>();
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

YOMM2_DECLARE(
    Subtype, times, (virtual_<const matrix&>, virtual_<const matrix&>));
YOMM2_DECLARE(Subtype, times, (double, virtual_<const matrix&>));
YOMM2_DECLARE(Subtype, times, (virtual_<const matrix&>, double));

YOMM2_DEFINE(Subtype, times, (const matrix&, const matrix&)) {
    return MATRIX_MATRIX;
}

YOMM2_DEFINE(Subtype, times, (const diagonal_matrix&, const diagonal_matrix&)) {
    return DIAGONAL_DIAGONAL;
}

YOMM2_DEFINE(Subtype, times, (double a, const matrix& m)) {
    return SCALAR_MATRIX;
}

YOMM2_DEFINE(Subtype, times, (double a, const diagonal_matrix& m)) {
    return SCALAR_DIAGONAL;
}

YOMM2_DEFINE(Subtype, times, (const diagonal_matrix& m, double a)) {
    return DIAGONAL_SCALAR;
}

YOMM2_DEFINE(Subtype, times, (const matrix& m, double a)) {
    return MATRIX_SCALAR;
}

YOMM2_DECLARE(int, times, (virtual_<matrix&&>, virtual_<matrix&&>));
YOMM2_DECLARE(int, times, (double, virtual_<matrix&&>));
YOMM2_DECLARE(int, times, (virtual_<matrix&&>, double));

YOMM2_DEFINE(int, times, (matrix&&, matrix&&)) {
    return -MATRIX_MATRIX;
}

YOMM2_DEFINE(int, times, (diagonal_matrix&&, diagonal_matrix&&)) {
    return -DIAGONAL_DIAGONAL;
}

YOMM2_DEFINE(int, times, (double a, matrix&& m)) {
    return -SCALAR_MATRIX;
}

YOMM2_DEFINE(int, times, (double a, diagonal_matrix&& m)) {
    return -SCALAR_DIAGONAL;
}

YOMM2_DEFINE(int, times, (diagonal_matrix && m, double a)) {
    return -DIAGONAL_SCALAR;
}

YOMM2_DEFINE(int, times, (matrix && m, double a)) {
    return -MATRIX_SCALAR;
}

YOMM2_DECLARE(Subtype, zero_ref, (virtual_<matrix&>));

YOMM2_DEFINE(Subtype, zero_ref, (dense_matrix & m)) {
    return MATRIX;
}

YOMM2_DEFINE(Subtype, zero_ref, (diagonal_matrix & m)) {
    return DIAGONAL;
}

YOMM2_DECLARE(Subtype, zero_ptr, (virtual_<matrix*>));

YOMM2_DEFINE(Subtype, zero_ptr, (dense_matrix * m)) {
    return MATRIX;
}

YOMM2_DEFINE(Subtype, zero_ptr, (diagonal_matrix * m)) {
    return DIAGONAL;
}

BOOST_AUTO_TEST_CASE(simple) {
    auto report = update();

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
        BOOST_TEST(zero_ptr(&dense) == MATRIX);
        diagonal_matrix diagonal;
        BOOST_TEST(zero_ref(diagonal) == DIAGONAL);
        BOOST_TEST(zero_ptr(&diagonal) == DIAGONAL);
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

struct YOMM2_SYMBOL(kick);
using kick = method<YOMM2_SYMBOL(kick), std::string(virtual_<Animal&>)>;

std::string kick_dog(Dog& dog) {
    return "bark";
}

YOMM2_STATIC(kick::override_fn<kick_dog>);

std::string kick_bulldog(Bulldog& dog) {
    return kick::next_fn<kick_bulldog>(dog) + " and bite back";
}

YOMM2_STATIC(kick::override_fn<kick_bulldog>);

BOOST_AUTO_TEST_CASE(test_next_fn) {
    update();

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

YOMM2_DECLARE(void, times, (virtual_<const matrix&>, virtual_<const matrix&>));

YOMM2_DEFINE(void, times, (const diagonal_matrix&, const matrix&)) {
}

YOMM2_DEFINE(void, times, (const matrix&, const diagonal_matrix&)) {
}

void test_handler(const error_type& error_v) {
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

BOOST_AUTO_TEST_CASE(call_error_handling) {
    update();

    auto prev_handler = set_error_handler(test_handler);

    try {
        times(dense_matrix(), dense_matrix());
        BOOST_FAIL("did not throw");
    } catch (const resolution_error& error) {
        BOOST_TEST(error.status == resolution_error::no_definition);
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }

    try {
        times(diagonal_matrix(), diagonal_matrix());
        BOOST_FAIL("did not throw");
    } catch (const resolution_error& error) {
        BOOST_TEST(error.status == resolution_error::ambiguous);
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }

#ifndef NDEBUG
    struct identity_matrix : matrix {};

    try {
        times(diagonal_matrix(), identity_matrix());
        BOOST_FAIL("did not throw");
    } catch (const unknown_class_error& error) {
        BOOST_TEST(
            error.type == reinterpret_cast<type_id>(&typeid(identity_matrix)));
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }
#endif

    set_error_handler(prev_handler);
}

} // namespace errors

namespace update_error_handling {

using test_policy = test_policy_<__COUNTER__>;

struct base {
    virtual ~base() {
    }
};

YOMM2_DECLARE(void, foo, (virtual_<base&>), test_policy);

BOOST_AUTO_TEST_CASE(test_update_error_handling) {
    auto prev_handler = test_policy::error;
    test_policy::error = errors::test_handler;

    try {
        update<test_policy>();
    } catch (const unknown_class_error& error) {
        test_policy::error = prev_handler;
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(base)));
        return;
    } catch (...) {
        test_policy::error = prev_handler;
        BOOST_FAIL("unexpected exception");
    }
    BOOST_FAIL("did not throw");
}
} // namespace update_error_handling

namespace across_namespaces {

namespace animals {

class Animal {
  public:
    virtual ~Animal() {
    }
};

YOMM2_DECLARE(std::string, kick, (virtual_<const Animal&>));

} // namespace animals

namespace more_animals {

class Dog : public animals::Animal {};

YOMM2_CLASSES(Dog, animals::Animal);

YOMM2_DEFINE(std::string, kick, (const Dog& dog)) {
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
    using kick = method<kick_, void(virtual_<Animal&>), test_policy>;
    using pet = method<pet_, void(virtual_<Animal&>), test_policy>;
    using meet =
        method<meet_, void(virtual_<Animal&>, virtual_<Animal&>), test_policy>;

    auto report = update<test_policy>().report;
    BOOST_TEST(report.not_implemented == 3);
    BOOST_TEST(report.ambiguous == 0);
    // 'meet' dispatch table is one cell, containing 'not_implemented'
    BOOST_TEST(report.cells == 1);

    YOMM2_STATIC(kick::override_fn<fn<Animal>>);
    report = update<test_policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    YOMM2_STATIC(pet::override_fn<fn<Cat>>);
    YOMM2_STATIC(pet::override_fn<fn<Dog>>);
    report = update<test_policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    // create ambiguity
    YOMM2_STATIC(meet::override_fn<fn<Animal, Cat>>);
    YOMM2_STATIC(meet::override_fn<fn<Dog, Animal>>);
    report = update<test_policy>().report;
    BOOST_TEST(report.cells == 4);
    BOOST_TEST(report.ambiguous == 1);

    YOMM2_STATIC(meet::override_fn<fn<Cat, Cat>>);
    report = update<test_policy>().report;
    BOOST_TEST(report.cells == 6);
    BOOST_TEST(report.ambiguous == 1);

    // shadow ambiguity
    YOMM2_STATIC(meet::override_fn<fn<Dog, Dog>>);
    YOMM2_STATIC(meet::override_fn<fn<Dog, Cat>>);
    YOMM2_STATIC(meet::override_fn<fn<Cat, Dog>>);
    report = update<test_policy>().report;
    BOOST_TEST(report.cells == 9);
    BOOST_TEST(report.ambiguous == 0);
}

} // namespace report
