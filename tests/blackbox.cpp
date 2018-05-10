// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using yorel::yomm2::virtual_;

namespace states {

using std::string;

struct Animal {
    Animal(const Animal&) = delete;
    Animal() : name("wrong") { }
    virtual ~Animal() {}
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

BOOST_AUTO_TEST_CASE(states) {
    yorel::yomm2::update_methods();
    const Animal& dog= Dog("spot");
    BOOST_TEST("dog spot" == name(dog));
    const Animal& cat= Cat("felix");
    BOOST_TEST("cat felix" == name(cat));
}

}

namespace matrices {

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

enum Subtype {
    SCALAR_MATRIX, SCALAR_DIAGONAL, MATRIX_SCALAR, DIAGONAL_SCALAR,
  MATRIX_MATRIX, DIAGONAL_DIAGONAL
};

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(Subtype, times, (virtual_<const matrix&>, virtual_<const matrix&>));
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

YOMM2_DEFINE(int, times, (diagonal_matrix&& m, double a)) {
    return -DIAGONAL_SCALAR;
}

YOMM2_DEFINE(int, times, (matrix&& m, double a)) {
    return -MATRIX_SCALAR;
}

BOOST_AUTO_TEST_CASE(simple)
{
    yorel::yomm2::update_methods();

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
        BOOST_TEST(times(diagonal_matrix(), diagonal_matrix()) == -DIAGONAL_DIAGONAL);
        BOOST_TEST(times(diagonal_matrix(), dense_matrix()) == -MATRIX_MATRIX);
        BOOST_TEST(times(2, dense_matrix()) == -SCALAR_MATRIX);
        BOOST_TEST(times(dense_matrix(), 2) == -MATRIX_SCALAR);
        BOOST_TEST(times(diagonal_matrix(), 2) == -DIAGONAL_SCALAR);
    }
}

}

namespace errors {

struct matrix {
    virtual ~matrix() {}
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

int error_code = -1;
YOMM2_TRACE(std::string method_name);

void test_handler(const yorel::yomm2::method_call_error& error) {
    error_code = error.code;
    YOMM2_TRACE(method_name = error.method_name);
}

BOOST_AUTO_TEST_CASE(error_handling)
{
    yorel::yomm2::update_methods();
    yorel::yomm2::set_method_call_error_handler(test_handler);
    times(dense_matrix(), dense_matrix());
    BOOST_TEST(error_code == yorel::yomm2::method_call_error::not_implemented);
    YOMM2_TRACE(
        BOOST_TEST(
            method_name == "times(virtual_<const matrix&>, virtual_<const matrix&>)"));
    YOMM2_TRACE(method_name = "");
    times(diagonal_matrix(), diagonal_matrix());
    BOOST_TEST(error_code == yorel::yomm2::method_call_error::ambiguous);
    YOMM2_TRACE(
        BOOST_TEST(
            method_name == "times(virtual_<const matrix&>, virtual_<const matrix&>)"));
}

}

namespace across_namespaces {

namespace animals {

class Animal {
  public:
    virtual ~Animal() {}
};

YOMM2_CLASS(Animal);

YOMM2_DECLARE(std::string, kick, (virtual_<const Animal&>));

}

namespace more_animals {

class Dog : public animals::Animal {};

YOMM2_CLASS(Dog, animals::Animal);

YOMM2_DEFINE(std::string, kick, (const Dog& dog)) {
  return "bark";
}

}

BOOST_AUTO_TEST_CASE(across_namespaces) {
    const animals::Animal& animal= more_animals::Dog();
    BOOST_TEST("bark" == kick(animal));
}

}

namespace refref {

struct Animal {
    virtual ~Animal() {}
    bool moved{false};
};

struct Dog : Animal {
};

struct Cat : virtual Animal {
};

YOMM2_CLASS(Animal);
YOMM2_CLASS(Dog, Animal);
YOMM2_CLASS(Cat, Animal);

YOMM2_DECLARE(void, test, (virtual_<Animal&&>));

YOMM2_DEFINE(void, test, (Dog&& dog)) {
    dog.moved = true;
}

YOMM2_DEFINE(void, test, (Cat&& cat)) {
    cat.moved = true;
}

BOOST_AUTO_TEST_CASE(states) {
    yorel::yomm2::update_methods();

    Dog dog;
    test(std::move(dog));
    BOOST_TEST(dog.moved);

    Cat cat;
    test(std::move(cat));
    BOOST_TEST(cat.moved);
}

}
