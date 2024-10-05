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

using policy = test_policy_<__COUNTER__>;
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

YOMM2_CLASSES(Animal, Dog, Cat, policy);

YOMM2_METHOD(name, (virtual_<const Animal&>), string, policy);

YOMM2_OVERRIDE(name, (const Cat& cat), string) {
    return "cat " + cat.name;
}

YOMM2_OVERRIDE(name, (const Dog& dog), string) {
    return "dog " + dog.name;
}

BOOST_AUTO_TEST_CASE(initializing) {
    initialize<policy>();
    const Animal& dog = Dog("spot");
    BOOST_TEST("dog spot" == name(dog));
    const Animal& cat = Cat("felix");
    BOOST_TEST("cat felix" == name(cat));
}

} // namespace states

namespace matrices {

using policy = test_policy_<__COUNTER__>;

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

YOMM2_CLASSES(matrix, dense_matrix, diagonal_matrix, policy);

YOMM2_METHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), Subtype, policy);
YOMM2_METHOD(times, (double, virtual_<const matrix&>), Subtype, policy);
YOMM2_METHOD(times, (virtual_<const matrix&>, double), Subtype, policy);

YOMM2_OVERRIDE(times, (const matrix&, const matrix&), Subtype) {
    return MATRIX_MATRIX;
}

YOMM2_OVERRIDE(
    times, (const diagonal_matrix&, const diagonal_matrix&), Subtype) {
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

YOMM2_METHOD(times, (virtual_<matrix&&>, virtual_<matrix&&>), int, policy);
YOMM2_METHOD(times, (double, virtual_<matrix&&>), int, policy);
YOMM2_METHOD(times, (virtual_<matrix&&>, double), int, policy);

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

YOMM2_METHOD(zero, (virtual_<matrix&>), Subtype, policy);

YOMM2_OVERRIDE(zero, (dense_matrix & m), Subtype) {
    return MATRIX;
}

YOMM2_OVERRIDE(zero, (diagonal_matrix & m), Subtype) {
    return DIAGONAL;
}

BOOST_AUTO_TEST_CASE(simple) {
    auto report = initialize<policy>();

    {
        // pass by const ref
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
        // pass by xref
        BOOST_TEST(times(dense_matrix(), dense_matrix()) == -MATRIX_MATRIX);
        BOOST_TEST(
            times(diagonal_matrix(), diagonal_matrix()) == -DIAGONAL_DIAGONAL);
        BOOST_TEST(times(diagonal_matrix(), dense_matrix()) == -MATRIX_MATRIX);
        BOOST_TEST(times(2, dense_matrix()) == -SCALAR_MATRIX);
        BOOST_TEST(times(dense_matrix(), 2) == -MATRIX_SCALAR);
        BOOST_TEST(times(diagonal_matrix(), 2) == -DIAGONAL_SCALAR);
    }

    {
        // pass by ref
        dense_matrix dense;
        BOOST_TEST(zero(dense) == MATRIX);
        diagonal_matrix diagonal;
        BOOST_TEST(zero(diagonal) == DIAGONAL);
    }
}

} // namespace matrices

namespace ambiguity {

using policy = test_policy_<__COUNTER__>;

struct matrix {
    virtual ~matrix() {
    }
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

enum Subtype { NONE, MATRIX_MATRIX, MATRIX_DIAGONAL, DIAGONAL_MATRIX };

YOMM2_CLASSES(matrix, dense_matrix, diagonal_matrix, policy);

YOMM2_METHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>),
    std::pair<Subtype, Subtype>, policy);

YOMM2_OVERRIDE(
    times, (const matrix&, const matrix&), std::pair<Subtype, Subtype>) {
    BOOST_TEST(!has_next());
    return std::pair(MATRIX_MATRIX, NONE);
}

YOMM2_OVERRIDE(
    times, (const matrix& a, const diagonal_matrix& b),
    std::pair<Subtype, Subtype>) {
    BOOST_TEST(has_next());
    return std::pair(MATRIX_DIAGONAL, next(a, b).first);
}

YOMM2_OVERRIDE(
    times, (const diagonal_matrix& a, const matrix& b),
    std::pair<Subtype, Subtype>) {
    BOOST_TEST(has_next());
    return std::pair(DIAGONAL_MATRIX, next(a, b).first);
}

BOOST_AUTO_TEST_CASE(ambiguity) {
    auto compiler = initialize<policy>();
    BOOST_TEST(compiler.report.ambiguous == 1);

    // N2216: in case of ambiguity, pick one.
    diagonal_matrix diag1, diag2;
    auto result1 = times(diag1, diag2);
    BOOST_TEST(result1.first == DIAGONAL_MATRIX);
    // Which overrider is picked is NOT documented! However, I know that it is
    // the last in registration order. This is important for the test for
    // ambiguity resolution using covariant return types.
    BOOST_TEST(result1.second == MATRIX_MATRIX);

    // but always the same
    auto result2 = times(diag1, diag2);
    BOOST_TEST((result1 == result2));
}

} // namespace ambiguity
namespace covariant_return_type {

using policy = test_policy_<__COUNTER__>;

enum Subtype { MATRIX_MATRIX, MATRIX_DENSE, DENSE_MATRIX };

struct matrix {
    virtual ~matrix() {
    }

    Subtype type;
};

struct dense_matrix : matrix {};

YOMM2_CLASSES(matrix, dense_matrix, policy);

YOMM2_METHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), matrix*, policy);

YOMM2_OVERRIDE(times, (const matrix&, const dense_matrix&), matrix*) {
    auto result = new dense_matrix;
    result->type = MATRIX_DENSE;
    return result;
}

YOMM2_OVERRIDE(times, (const dense_matrix&, const matrix&), dense_matrix*) {
    auto result = new dense_matrix;
    result->type = DENSE_MATRIX;
    return result;
}

BOOST_AUTO_TEST_CASE(covariant_return_type) {
    auto compiler = initialize<policy>();
    BOOST_TEST(compiler.report.ambiguous == 0);

    // N2216: use covariant return types to resolve ambiguity.
    dense_matrix left, right;
    std::unique_ptr<matrix> result(times(left, right));
    BOOST_TEST(result->type == DENSE_MATRIX);
}

} // namespace covariant_return_type

namespace test_next_fn {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};
struct Bulldog : Dog {};

register_classes(Animal, Dog, Bulldog);

struct YOMM2_METHOD_NAME(poke);
using poke = method<YOMM2_METHOD_NAME(poke)(virtual_<Animal&>), std::string>;

std::string poke_dog(Dog& dog) {
    return "bark";
}

YOMM2_REGISTER(poke::override<poke_dog>);

std::string poke_bulldog(Bulldog& dog) {
    return poke::next<poke_bulldog>(dog) + " and bite back";
}

YOMM2_REGISTER(poke::override<poke_bulldog>);

BOOST_AUTO_TEST_CASE(test_next_fn) {
    initialize();

    std::unique_ptr<Animal> snoopy = std::make_unique<Dog>();
    BOOST_TEST(poke::fn(*snoopy) == "bark");

    std::unique_ptr<Animal> hector = std::make_unique<Bulldog>();
    BOOST_TEST(poke::fn(*hector) == "bark and bite back");
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

using policy = test_policy_<__COUNTER__>;

struct base {
    virtual ~base() {
    }
};

YOMM2_METHOD(foo, (virtual_<base&>), void, policy);

BOOST_AUTO_TEST_CASE(test_initialize_error_handling) {
    auto prev_handler = policy::set_error_handler(errors::test_handler);

    try {
        initialize<policy>();
    } catch (const unknown_class_error& error) {
        policy::set_error_handler(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(base)));
        return;
    } catch (...) {
        policy::set_error_handler(prev_handler);
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

YOMM2_METHOD(poke, (virtual_<const Animal&>), std::string);

} // namespace animals

namespace more_animals {

class Dog : public animals::Animal {};

YOMM2_CLASSES(Dog, animals::Animal);

YOMM2_OVERRIDE(poke, (const Dog& dog), std::string) {
    return "bark";
}

} // namespace more_animals

BOOST_AUTO_TEST_CASE(across_namespaces) {
    const animals::Animal& animal = more_animals::Dog();
    BOOST_TEST("bark" == poke(animal));
}

} // namespace across_namespaces

namespace report {

using policy = test_policy_<__COUNTER__>;

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

struct poke_;
struct pet_;
struct meet_;

template<class... Class>
void fn(Class&...) {
}

YOMM2_CLASSES(Animal, Dog, Cat, policy);

BOOST_AUTO_TEST_CASE(initialize_report) {
    using poke = method<poke_(virtual_<Animal&>), void, policy>;
    using pet = method<pet_(virtual_<Animal&>), void, policy>;
    using meet =
        method<meet_(virtual_<Animal&>, virtual_<Animal&>), void, policy>;

    auto report = initialize<policy>().report;
    BOOST_TEST(report.not_implemented == 3);
    BOOST_TEST(report.ambiguous == 0);
    // 'meet' dispatch table is one cell, containing 'not_implemented'
    BOOST_TEST(report.cells == 1);

    YOMM2_REGISTER(poke::override<fn<Animal>>);
    report = initialize<policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    YOMM2_REGISTER(pet::override<fn<Cat>>);
    YOMM2_REGISTER(pet::override<fn<Dog>>);
    report = initialize<policy>().report;
    BOOST_TEST(report.not_implemented == 2);

    // create ambiguity
    YOMM2_REGISTER(meet::override<fn<Animal, Cat>>);
    YOMM2_REGISTER(meet::override<fn<Dog, Animal>>);
    report = initialize<policy>().report;
    BOOST_TEST(report.cells == 4);
    BOOST_TEST(report.ambiguous == 1);

    YOMM2_REGISTER(meet::override<fn<Cat, Cat>>);
    report = initialize<policy>().report;
    BOOST_TEST(report.cells == 6);
    BOOST_TEST(report.ambiguous == 1);

    // shadow ambiguity
    YOMM2_REGISTER(meet::override<fn<Dog, Dog>>);
    YOMM2_REGISTER(meet::override<fn<Dog, Cat>>);
    YOMM2_REGISTER(meet::override<fn<Cat, Dog>>);
    report = initialize<policy>().report;
    BOOST_TEST(report.cells == 9);
    BOOST_TEST(report.ambiguous == 0);
}

} // namespace report

namespace test_comma_in_return_type {

using policy = test_policy_<__COUNTER__>;

struct Test {
    virtual ~Test() {};
};

YOMM2_CLASSES(Test, policy);

YOMM2_METHOD(foo, (virtual_<Test&>), std::pair<int, int>, policy);

YOMM2_OVERRIDE(foo, (Test&), std::pair<int, int>) {
    return {1, 2};
}

BOOST_AUTO_TEST_CASE(comma_in_return_type) {
    initialize<policy>();

    Test test;

    BOOST_TEST((foo(test) == std::pair(1, 2)));
}

} // namespace test_comma_in_return_type
