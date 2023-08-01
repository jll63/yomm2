// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;
using yorel::yomm2::detail::types;

namespace states {

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

YOMM2_CLASS(Animal);
YOMM2_CLASS(Dog, Animal);
YOMM2_CLASS(Cat, Animal);

YOMM2_DECLARE(string, name, (virtual_<const Animal&>));

YOMM2_DEFINE(string, name, (const Dog& dog)) {
    return "dog " + dog.name;
}

YOMM2_DEFINE(string, name, (const Cat& cat)) {
    return "cat " + cat.name;
}

BOOST_AUTO_TEST_CASE(initializing) {
    update();
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

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

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
    update();

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

namespace errors {

struct matrix {
    virtual ~matrix() {
    }
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(void, times, (virtual_<const matrix&>, virtual_<const matrix&>));

YOMM2_DEFINE(void, times, (const diagonal_matrix&, const matrix&)) {
}

YOMM2_DEFINE(void, times, (const matrix&, const diagonal_matrix&)) {
}

void deprecated_test_handler(
    const method_call_error& error, size_t, const std::type_info* const*) {
    throw error;
}

BOOST_AUTO_TEST_CASE(deprecated_error_handling) {
    update();
    set_method_call_error_handler(deprecated_test_handler);

    try {
        times(dense_matrix(), dense_matrix());
    } catch (const method_call_error& error) {
        BOOST_TEST(error.code == method_call_error::not_implemented);
        return;
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }

    try {
        times(diagonal_matrix(), diagonal_matrix());
    } catch (const method_call_error& error) {
        BOOST_TEST(error.code == method_call_error::ambiguous);
        return;
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }
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

    set_error_handler(test_handler);

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
        BOOST_TEST(error.ti == &typeid(identity_matrix));
    } catch (...) {
        BOOST_FAIL("unexpected exception");
    }
#endif
}

} // namespace errors

namespace update_error_handling {

struct test_policy : default_policy {
    static struct catalog catalog;
    static struct context context;
};

catalog test_policy::catalog;
context test_policy::context;

struct base {
    virtual ~base() {
    }
};

struct derived : base {};

class_declaration<types<derived, base>, test_policy> YOMM2_GENSYM;

BOOST_AUTO_TEST_CASE(test_update_error_handling) {
    try {
        update<test_policy>();
    } catch (const unknown_class_error& error) {
        BOOST_TEST(error.ti == &typeid(base));
        return;
    } catch (...) {
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

YOMM2_CLASS(Animal);

YOMM2_DECLARE(std::string, kick, (virtual_<const Animal&>));

} // namespace animals

namespace more_animals {

class Dog : public animals::Animal {};

YOMM2_CLASS(Dog, animals::Animal);

YOMM2_DEFINE(std::string, kick, (const Dog& dog)) {
    return "bark";
}

} // namespace more_animals

BOOST_AUTO_TEST_CASE(across_namespaces) {
    const animals::Animal& animal = more_animals::Dog();
    BOOST_TEST("bark" == kick(animal));
}

} // namespace across_namespaces

namespace refref {

struct Animal {
    virtual ~Animal() {
    }
    bool moved{false};
};

struct Dog : Animal {};

struct Cat : virtual Animal {};

YOMM2_CLASS(Animal);
YOMM2_CLASS(Dog, Animal);
YOMM2_CLASS(Cat, Animal);

YOMM2_DECLARE(void, test, (virtual_<Animal&&>));

YOMM2_DEFINE(void, test, (Dog && dog)) {
    dog.moved = true;
}

YOMM2_DEFINE(void, test, (Cat && cat)) {
    cat.moved = true;
}

BOOST_AUTO_TEST_CASE(moving) {
    update();

    Dog dog;
    test(std::move(dog));
    BOOST_TEST(dog.moved);

    Cat cat;
    test(std::move(cat));
    BOOST_TEST(cat.moved);
}

} // namespace refref
