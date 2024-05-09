// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>

#include <iostream>
#include <memory>
#include <string>

struct Asteroid;
struct Object;
struct Rocket;
namespace animals {
struct Animal;
struct YoMm2_S_kick;
struct YoMm2_S_meet;
namespace pets {
struct Cat;
struct Dog;
} // namespace pets
} // namespace animals
namespace yorel {
namespace yomm2 {
namespace detail {
template<>
struct static_offsets<yorel::yomm2::method<
    animals::YoMm2_S_kick,
    int(yorel::yomm2::virtual_ptr<
        animals::Animal, yorel::yomm2::policy::debug_shared>),
    yorel::yomm2::policy::debug_shared>> {
    static constexpr size_t slots[] = {0};
};
template<>
struct static_offsets<yorel::yomm2::method<
    animals::YoMm2_S_meet,
    int(yorel::yomm2::virtual_ptr<
            animals::Animal, yorel::yomm2::policy::debug_shared>,
        yorel::yomm2::virtual_ptr<
            animals::Animal, yorel::yomm2::policy::debug_shared>),
    yorel::yomm2::policy::debug_shared>> {
    static constexpr size_t slots[] = {1, 2};
    static constexpr size_t strides[] = {2};
};
} // namespace detail
} // namespace yomm2
} // namespace yorel
namespace yorel {
namespace yomm2 {

struct node {};

using node_map = std::unordered_map<std::string, std::unique_ptr<node>>;

struct namespace_ : node {
    std::string name;
    node_map children;
};

struct class_ : node {
    std::string name;
};

} // namespace yomm2
} // namespace yorel

using std::cout;

using namespace yorel::yomm2;
using yorel::yomm2::virtual_ptr;

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

define_method(int, meet, (virtual_ptr<Animal> a, virtual_ptr<Animal> b)) {
    std::cout << "ignore\n";
    return 1;
}

define_method(int, meet, (virtual_ptr<Dog> a, virtual_ptr<Dog> b)) {
    std::cout << "wag tail\n";
    return 2;
}
} // namespace animals

struct Object {
    virtual ~Object() {
    }
};
struct Rocket : Object {};
struct Asteroid : Object {};

register_classes(Object, Rocket, Asteroid);

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

using namespace yorel::yomm2;

// static_assert(detail::has_static_offsets<
//               method<YOMM2_SYMBOL(kick), int(virtual_ptr<Animal>)>>::value);

// static_assert(detail::has_static_offsets<method<
//                   YOMM2_SYMBOL(meet),
//                   int(virtual_ptr<Animal>, virtual_ptr<Animal>)>>::value);

// void initialize() {
//     default_policy::static_vptr<Dog> = nullptr;
// }

int main() {
    update();

    using namespace animals;
    Dog snoopy, hector;
    Cat felix;

    // default_policy::trace_enabled = true;
    compiler<default_policy> comp;
    comp.compile();
    comp.install_global_tables();
    comp.generate_static_offsets(std::cout);

    kick(snoopy);
    kick(felix);
    meet(felix, snoopy);
    meet(hector, snoopy);

    // initialize();
    // std::cout << call_kick(snoopy) << "\n";

    return 0;
}
