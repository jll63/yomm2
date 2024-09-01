#include <iostream>
#include <type_traits>

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/detail/compiler.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE compiler
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;

using class_ = generic_compiler::class_;
using cc_method = generic_compiler::method;
using definition = generic_compiler::definition;

std::ostream& operator<<(std::ostream& os, const class_* cls) {
    return os
        << reinterpret_cast<const std::type_info*>(cls->type_ids[0])->name();
}

std::string empty = "{}";

template<template<typename...> typename Container, typename T>
auto str(const Container<T>& container) {
    std::ostringstream os;
    os << "{";
    const char* sep = "";

    for (const auto& item : container) {
        os << sep << item;
        sep = ", ";
    }

    os << "}";

    return os.str();
}

template<typename... Ts>
auto sstr(Ts... args) {
    std::vector<class_*> vec{args...};
    std::sort(vec.begin(), vec.end());
    return str(vec);
}

template<typename T>
auto sstr(const std::unordered_set<T>& container) {
    return sstr(std::vector<T>(container.begin(), container.end()));
}

template<typename T, typename Compiler>
auto get_class(const Compiler& comp) {
    return comp.class_map.at(typeid(T));
}

// A   B
//  \ / \
//  AB   D
//  |    |
//  C    E
//   \  /
//    F

struct A {
    virtual ~A() {
    }
};

struct B {
    virtual ~B() {
    }
};

struct AB : A, B {};

struct C : AB {};

struct D : B {};

struct E : D {};

struct F : C, E {};

// ============================================================================
// Test use_classes.

BOOST_AUTO_TEST_CASE(test_use_classes) {
    using test_policy = test_policy_<__COUNTER__>;
    YOMM2_STATIC(use_classes<A, B, AB, C, D, E, test_policy>);

    std::vector<class_*> actual, expected;

    auto comp = update<test_policy>();

    auto a = get_class<A>(comp);
    auto b = get_class<B>(comp);
    auto ab = get_class<AB>(comp);
    auto c = get_class<C>(comp);
    auto d = get_class<D>(comp);
    auto e = get_class<E>(comp);

    // -----------------------------------------------------------------------
    // A
    BOOST_REQUIRE_EQUAL(sstr(a->direct_bases), empty);
    BOOST_REQUIRE_EQUAL(sstr(a->direct_derived), sstr(ab));
    BOOST_REQUIRE_EQUAL(sstr(a->covariant_classes), sstr(a, ab, c));

    // -----------------------------------------------------------------------
    // B
    BOOST_REQUIRE_EQUAL(sstr(b->direct_bases), empty);
    BOOST_REQUIRE_EQUAL(sstr(b->direct_derived), sstr(ab, d));
    BOOST_REQUIRE_EQUAL(sstr(b->covariant_classes), sstr(b, ab, c, d, e));

    // -----------------------------------------------------------------------
    // AB
    BOOST_REQUIRE_EQUAL(sstr(ab->direct_bases), sstr(a, b));
    BOOST_REQUIRE_EQUAL(sstr(ab->direct_derived), sstr(c));
    BOOST_REQUIRE_EQUAL(sstr(ab->covariant_classes), sstr(ab, c));

    // -----------------------------------------------------------------------
    // C
    BOOST_REQUIRE_EQUAL(sstr(c->direct_bases), sstr(ab));
    BOOST_REQUIRE_EQUAL(sstr(c->direct_derived), empty);
    BOOST_REQUIRE_EQUAL(sstr(c->covariant_classes), sstr(c));

    // -----------------------------------------------------------------------
    // D
    BOOST_REQUIRE_EQUAL(sstr(d->direct_bases), sstr(b));
    BOOST_REQUIRE_EQUAL(sstr(d->direct_derived), sstr(e));
    BOOST_REQUIRE_EQUAL(sstr(d->covariant_classes), sstr(d, e));

    // -----------------------------------------------------------------------
    // E
    BOOST_REQUIRE_EQUAL(sstr(e->direct_bases), sstr(d));
    BOOST_REQUIRE_EQUAL(sstr(e->direct_derived), empty);
    BOOST_REQUIRE_EQUAL(sstr(e->covariant_classes), sstr(e));
}

/// ============================================================================
// Test assign_slots.

template<typename Compiler>
const auto& get_method(const Compiler& comp, const method_info& info) {
    for (const auto& m : comp.methods) {
        if (m.info == &info) {
            return m;
        }
    }

    BOOST_FAIL("method not found");

    return comp.methods.front();
}

template<int>
struct M;

#define ADD_METHOD(CLASS)                                                      \
    auto& BOOST_PP_CAT(m_, CLASS) =                                            \
        method<CLASS, void(virtual_<CLASS&>), test_policy>::fn;

#define ADD_METHOD_N(CLASS, N)                                                 \
    auto& BOOST_PP_CAT(BOOST_PP_CAT(m_, CLASS), N) =                           \
        method<M<N>, void(virtual_<CLASS&>), test_policy>::fn;

BOOST_AUTO_TEST_CASE(test_assign_slots_a_b1_c) {
    using test_policy = test_policy_<__COUNTER__>;

    //   A
    //  / \
    // B1  C

    struct A {};
    struct B : A {};
    struct C : A {};

    YOMM2_STATIC(use_classes<A, test_policy>);
    YOMM2_STATIC(use_classes<A, B, test_policy>);
    YOMM2_STATIC(use_classes<A, C, test_policy>);
    ADD_METHOD(B);
    auto comp = update<test_policy>();

    BOOST_TEST_REQUIRE(get_method(comp, m_B).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_B).slots[0] == 0);
    BOOST_TEST(get_class<A>(comp)->vtbl.size() == 0);
    BOOST_TEST(get_class<B>(comp)->vtbl.size() == 1);
    BOOST_TEST(get_class<C>(comp)->vtbl.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_assign_slots_a1_b1_c1) {
    using test_policy = test_policy_<__COUNTER__>;

    //   A1
    //  / \
    // B1  C1

    struct A {};
    struct B : A {};
    struct C : A {};

    YOMM2_STATIC(use_classes<A, test_policy>);
    YOMM2_STATIC(use_classes<A, B, test_policy>);
    YOMM2_STATIC(use_classes<A, C, test_policy>);
    ADD_METHOD(A);
    ADD_METHOD(B);
    ADD_METHOD(C);
    auto comp = update<test_policy>();

    BOOST_TEST_REQUIRE(get_method(comp, m_A).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_A).slots[0] == 0);
    BOOST_TEST(get_class<A>(comp)->vtbl.size() == 1);

    BOOST_TEST_REQUIRE(get_method(comp, m_B).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_B).slots[0] == 1);
    BOOST_TEST(get_class<B>(comp)->vtbl.size() == 2);

    BOOST_TEST_REQUIRE(get_method(comp, m_C).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_C).slots[0] == 1);
    BOOST_TEST(get_class<C>(comp)->vtbl.size() == 2);
}

BOOST_AUTO_TEST_CASE(test_assign_slots_a1_b1_d1_c1_d1) {
    using test_policy = test_policy_<__COUNTER__>;

    //   A1
    //  / \
    // B1  C1 - slots 0-2 are wasted
    //  \ /
    //   D1

    struct A {};
    struct B : virtual A {};
    struct C : virtual A {};
    struct D : B, C {};

    YOMM2_STATIC(use_classes<A, test_policy>);
    YOMM2_STATIC(use_classes<A, B, test_policy>);
    YOMM2_STATIC(use_classes<A, C, test_policy>);
    YOMM2_STATIC(use_classes<D, B, C, test_policy>);
    ADD_METHOD(A);
    ADD_METHOD(B);
    ADD_METHOD(C);
    ADD_METHOD(D);
    auto comp = update<test_policy>();

    BOOST_TEST_REQUIRE(get_method(comp, m_A).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_A).slots[0] == 0);
    BOOST_TEST(get_class<A>(comp)->vtbl.size() == 1);

    BOOST_TEST_REQUIRE(get_method(comp, m_B).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_B).slots[0] == 1);
    BOOST_TEST(get_class<B>(comp)->vtbl.size() == 2);

    BOOST_TEST_REQUIRE(get_method(comp, m_D).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_D).slots[0] == 2);
    BOOST_TEST(get_class<D>(comp)->vtbl.size() == 4);

    BOOST_TEST_REQUIRE(get_method(comp, m_C).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_C).slots[0] == 3);
    BOOST_TEST(get_class<C>(comp)->vtbl.size() == 4);
    // slots 0-2 in C are wasted, to make room for methods in B and D
}

BOOST_AUTO_TEST_CASE(test_assign_slots_a1_b1_d1_c1_d1_e2) {
    using test_policy = test_policy_<__COUNTER__>;

    //   A1
    //  / \
    // B1  C1 slots 0-2 are wasted
    //  \ /  \
    //   D1  E2 but E can use them

    struct A {};
    struct B : virtual A {};
    struct C : virtual A {};
    struct E : C {};
    struct D : B, C {};

    YOMM2_STATIC(use_classes<A, test_policy>);
    YOMM2_STATIC(use_classes<A, B, test_policy>);
    YOMM2_STATIC(use_classes<A, C, test_policy>);
    YOMM2_STATIC(use_classes<C, E, test_policy>);
    YOMM2_STATIC(use_classes<D, B, C, test_policy>);
    ADD_METHOD(A);
    ADD_METHOD(B);
    ADD_METHOD(C);
    ADD_METHOD(D);
    ADD_METHOD_N(E, 1);
    ADD_METHOD_N(E, 2);
    ADD_METHOD_N(E, 3);
    auto comp = update<test_policy>();

    BOOST_TEST_REQUIRE(get_method(comp, m_A).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_A).slots[0] == 0);
    BOOST_TEST(get_class<A>(comp)->vtbl.size() == 1);

    BOOST_TEST_REQUIRE(get_method(comp, m_B).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_B).slots[0] == 1);
    BOOST_TEST(get_class<B>(comp)->vtbl.size() == 2);

    BOOST_TEST_REQUIRE(get_method(comp, m_D).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_D).slots[0] == 2);
    BOOST_TEST(get_class<D>(comp)->vtbl.size() == 4);

    BOOST_TEST_REQUIRE(get_method(comp, m_C).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_C).slots[0] == 3);
    BOOST_TEST(get_class<C>(comp)->vtbl.size() == 4);
    // slots 0-2 in C are wasted, to make room for methods in B and D

    BOOST_TEST_REQUIRE(get_method(comp, m_E1).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_E1).slots[0] == 1);

    BOOST_TEST_REQUIRE(get_method(comp, m_E2).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_E2).slots[0] == 2);

    BOOST_TEST_REQUIRE(get_method(comp, m_E3).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_E3).slots[0] == 4);

    BOOST_TEST(get_class<E>(comp)->vtbl.size() == 5);
}

BOOST_AUTO_TEST_CASE(test_assign_slots_a1_c1_b1) {
    using test_policy = test_policy_<__COUNTER__>;

    // A1  B1
    //  \  /
    //   C1

    struct A {};
    struct B {};
    struct C : A, B {};

    YOMM2_STATIC(use_classes<A, test_policy>);
    YOMM2_STATIC(use_classes<B, test_policy>);
    YOMM2_STATIC(use_classes<A, B, C, test_policy>);
    ADD_METHOD(A);
    ADD_METHOD(B);
    ADD_METHOD(C);
    auto comp = update<test_policy>();

    BOOST_TEST_REQUIRE(get_method(comp, m_A).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_A).slots[0] == 0);
    BOOST_TEST(get_class<A>(comp)->vtbl.size() == 1);

    BOOST_TEST_REQUIRE(get_method(comp, m_C).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_C).slots[0] == 1);
    BOOST_TEST(get_class<C>(comp)->vtbl.size() == 3);

    BOOST_TEST_REQUIRE(get_method(comp, m_B).slots.size() == 1);
    BOOST_TEST(get_method(comp, m_B).slots[0] == 2);
    BOOST_TEST(get_class<B>(comp)->first_slot == 2);
    BOOST_TEST(get_class<B>(comp)->vtbl.size() == 1);
}
