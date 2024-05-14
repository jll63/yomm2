// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/generator.hpp>

#include <iostream>
#include <memory>
#include <string>

using std::cout;

using namespace yorel::yomm2;
using yorel::yomm2::virtual_ptr;

struct Object {
    virtual ~Object() {
    }
};

register_classes(Object);

namespace animals {
struct Animal {
    virtual ~Animal() {
    }
};

namespace pets {
struct Cat : Animal {};
struct Dog : Animal {};
} // namespace pets

using namespace pets;

struct YOMM2_SYMBOL(kick);
struct YOMM2_SYMBOL(meet);

register_classes(Animal, Dog, Cat);

declare_method(int, kick, (virtual_ptr<Animal>));

define_method(int, kick, (virtual_ptr<Cat> dog)) {
    std::cout << "hiss\n";
    return 0;
}

define_method(int, kick, (virtual_ptr<Dog> dog)) {
    std::cout << "bark\n";
    return 0;
}

declare_method(int, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>));

template<int>
struct X {};

declare_method(int, test, (virtual_ptr<Animal>, X<42>&));

define_method(int, meet, (virtual_ptr<Animal> a, virtual_ptr<Animal> b)) {
    std::cout << "ignore\n";
    return 1;
}

define_method(int, meet, (virtual_ptr<Dog> a, virtual_ptr<Dog> b)) {
    std::cout << "wag tail\n";
    return 2;
}
} // namespace animals

namespace game {
struct Rocket : Object {};
struct Asteroid : Object {};
register_classes(Object, Rocket, Asteroid);
} // namespace game

// namespace yorel {
// namespace yomm2 {
// namespace detail {
// template<>
// struct static_offsets<method<YOMM2_SYMBOL(kick), int(virtual_ptr<Animal>)>> {
//     static constexpr size_t slots[] = {0};
// };

// template<>
// struct static_offsets<
//     method<YOMM2_SYMBOL(meet), int(virtual_ptr<Animal>, virtual_ptr<Animal>)>> {
//     static constexpr size_t slots[] = {1, 2};
//     static constexpr size_t strides[] = {3};
// };

// } // namespace detail
// } // namespace yomm2
// } // namespace yorel

template<>
struct ::yorel::yomm2::detail::static_offsets<
    method<animals::YOMM2_SYMBOL(kick), int(virtual_ptr<animals::Animal>)>> {
    static constexpr size_t slots[] = {0};
};

using namespace yorel::yomm2;

// static_assert(detail::has_static_offsets<
//               method<YOMM2_SYMBOL(kick), int(virtual_ptr<Animal>)>>::value);

// static_assert(detail::has_static_offsets<method<
//                   YOMM2_SYMBOL(meet),
//                   int(virtual_ptr<Animal>, virtual_ptr<Animal>)>>::value);

// void initialize() {
//     default_policy::static_vptr<Dog> = nullptr;
// }

template<typename>
struct foo {};

typedef foo<int> bar;

int main() {
    std::cout << boost::core::demangle(typeid(std::ostream).name()) << "\n";
    std::cout << boost::core::demangle(typeid(bar).name()) << "\n";
    return 0;
    update();

    using namespace animals;
    Dog snoopy, hector;
    Cat felix;

    // default_policy::trace_enabled = true;
    compiler<default_policy> comp;
    comp.compile();
    comp.install_global_tables();

    generator gen(comp, std::cout);
    gen.write_forward_declarations();
    gen.write_static_offsets();

    kick(snoopy);
    kick(felix);
    meet(felix, snoopy);
    meet(hector, snoopy);

    // initialize();
    // std::cout << call_kick(snoopy) << "\n";

    return 0;
}
