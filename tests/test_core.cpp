// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;
using namespace boost::mp11;

// clang-format off

namespace YOMM2_GENSYM {

struct base {
    virtual ~base() {}
};

struct a : base {};
struct b : base {};
struct c : base {};
struct d : base {};
struct e : base {};
struct f : base {};

static_assert(
    std::is_same_v<
        mp_filter<
            is_virtual,
            types< virtual_<a&>, b, virtual_<c&> >
        >,
        types< virtual_<a&>, virtual_<c&> >
    >);

static_assert(
    std::is_same_v<
        remove_virtual<virtual_<a&>>,
        a&
    >);

static_assert(
    std::is_same_v<
        polymorphic_type<default_policy, a&>,
        a
    >);

static_assert(
    std::is_same_v<
        mp_transform<
            remove_virtual,
            types< virtual_<a&>, virtual_<c&> >
        >,
        types<a&, c&>
    >);

static_assert(
    std::is_same_v<
        mp_transform_q<
            mp_bind_front<polymorphic_type, default_policy>,
            mp_transform<
                remove_virtual,
                types< virtual_<a&>, virtual_<c&> >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        mp_transform_q<
            mp_bind_front<polymorphic_type, default_policy>,
            mp_transform<
                remove_virtual,
                mp_filter<
                    is_virtual,
                    types< virtual_<a&>, b, virtual_<c&> >
                >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        polymorphic_types<types<virtual_<a&>, b, virtual_<c&>>>,
        types<a&, c&>>);

static_assert(
    std::is_same_v<
        polymorphic_types<types<
            virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>>,
        types<std::shared_ptr<a>, std::shared_ptr<c>>>);

static_assert(
    std::is_same_v<
        spec_polymorphic_types<
            default_policy,
            types<virtual_<a&>, b, virtual_<c&>>,
            types<d&, e, f&>>,
        types<d, f>>);

static_assert(
    std::is_same_v<
        polymorphic_type<default_policy, std::shared_ptr<a>>,
    a>);

static_assert(
    std::is_same_v<
        spec_polymorphic_types<
            default_policy,
            types<
                virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>,
            types<std::shared_ptr<d>, e, std::shared_ptr<f>>>,
        types<d, f>>);

BOOST_AUTO_TEST_CASE(test_type_id_list) {
    type_id expected[] = {type_id(&typeid(a)), type_id(&typeid(b))};
    auto iter = type_id_list<default_policy, types<a&, b&>>::begin;
    auto last = type_id_list<default_policy, types<a&, b&>>::end;
    BOOST_TEST_REQUIRE(last - iter == 2);
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(a)));
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(b)));
}

} // namespace YOMM2_GENSYM

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

const void* mammal_this(const Mammal& obj) {
    return &obj;
}

const void* carnivore_this(const Carnivore& obj) {
    return &obj;
}

const void* dog_this(const Dog& obj) {
    return &obj;
}

BOOST_AUTO_TEST_CASE(casts) {
    Dog dog;
    const Animal& animal = dog;
    const Mammal& mammal = dog;
    const Carnivore& carnivore = dog;

    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Carnivore&>(animal).c)
        == &dog.c);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Dog&>(animal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Mammal&>::cast<const Dog&>(mammal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Carnivore&>::cast<const Dog&>(carnivore).c)
        == &dog.c);

    using voidp = const void*;
    using virtual_animal_t = polymorphic_type<default_policy, const Animal&>;
    static_assert(std::is_same_v<virtual_animal_t, Animal>, "animal");
    using virtual_mammal_t = polymorphic_type<default_policy, const Mammal&>;
    static_assert(std::is_same_v<virtual_mammal_t, Mammal>, "mammal");

    voidp base_address;

    base_address = wrapper<
        default_policy,
        voidp(virtual_<const Animal&>), mammal_this,
        types<const Mammal&>>::fn(animal);
    BOOST_TEST(base_address == &mammal);

    base_address = wrapper<
        default_policy,
        voidp(virtual_<const Animal&>), carnivore_this,
        types<const Carnivore&>>::fn(animal);
    BOOST_TEST(base_address == &carnivore);

    base_address = wrapper<
        default_policy,
        voidp(virtual_<const Animal&>), mammal_this,
        types<const Dog&>>::fn(animal);
    BOOST_TEST(base_address == &dog);
}

} // namespace casts

namespace test_use_classes {

struct Animal {};
struct Dog : public Animal {};
struct Bulldog : public Dog {};
struct Cat : public Animal {};
struct Dolphin : public Animal {};

static_assert(
    std::is_same_v<
        inheritance_map<Animal, Dog, Bulldog, Cat, Dolphin>,
        types<
            types<Animal, Animal>,
            types<Dog, Animal, Dog>,
            types<Bulldog, Animal, Dog, Bulldog>,
            types<Cat, Animal, Cat>,
            types<Dolphin, Animal, Dolphin>
        >
>);

} // namespace test_use_classes

namespace facets {

using namespace policy;

struct key1;
struct key2;
struct alt_rtti {};

static_assert(std::is_same_v<
    copy_facet<key2, generic_domain<key1>>::type,
    generic_domain<key2>
>);

// yorel::yomm2::policy::generic_policy<facets::key2, yorel::yomm2::policy::std_rtti>,
// yorel::yomm2::policy::generic_policy<yorel::yomm2::policy::generic_domain<facets::key2>, yorel::yomm2::policy::std_rtti>

struct policy1 : generic_policy<policy1, generic_domain<policy1>, std_rtti> {};
struct policy2 : policy1::copy<policy2> {};
struct policy3 : policy1::copy<policy3>::replace<std_rtti, alt_rtti> {};

static_assert(std::is_same_v<
    policy2::facets,
    types<generic_domain<policy2>, std_rtti>
>);

static_assert(std::is_same_v<
    policy3::facets,
    types<generic_domain<policy3>, alt_rtti>
>);

// static_assert(std::is_same_v<
//     generic_policy<generic_domain<key1>, std_rtti>::replace<std_rtti, alt_rtti>,
//     generic_policy<generic_domain<key1>, alt_rtti>
// >);

}

void f(char, int) {}

static_assert(std::is_same_v<
    parameter_type_list_t<decltype(f)>,
    types<char, int>
>);
