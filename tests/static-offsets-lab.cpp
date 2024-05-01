// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <boost/core/demangle.hpp>

using std::cout;

using namespace yorel::yomm2;

struct Animal {
    virtual ~Animal() {
    }
};

struct Cat : Animal {};
struct Dog : Animal {};

struct YOMM2_SYMBOL(kick);
struct YOMM2_SYMBOL(meet);

using yorel::yomm2::virtual_ptr;

namespace yorel {
namespace yomm2 {
namespace detail {

template<class Policy>
template<typename Stream>
void runtime<Policy>::generate(Stream& os) {
    for (auto& cls : classes) {
        os << boost::core::demangle(
                  reinterpret_cast<const std::type_info*>(cls.type_ids[0])
                      ->name())
           << "\n";
        for (auto slot : cls.vtbl) {
            os << "  " << slot << "\n";
        }
        os << "\n";
    }

    //std::vector

    for (auto& m : methods) {
        auto slot = m.slots[0];

        if (m.arity() == 1) {
            for (auto cls : m.vp[0]->compatible_classes) {
                auto def = m.dispatch_table[cls->vtbl[slot]];
                auto x = m.dispatch_table;
                auto y = x[cls->vtbl[slot]];
                auto name = def == &m.ambiguous ? "ambiguous"
                    : def == &m.not_implemented
                    ? "not_implemented"
                    : boost::core::demangle(
                          reinterpret_cast<const std::type_info*>(
                              def->info->type)
                              ->name());
                os << boost::core::demangle(std::string(m.info->name).c_str())
                   << " slot: " << slot << ", offset: " << cls->vtbl[slot]
                   << ": " << name << "\n";
            }
        }
    }
}

template<>
struct static_offsets<method<YOMM2_SYMBOL(kick), int(virtual_ptr<Animal>)>> {
    static constexpr size_t slots[] = {0};
};

template<>
struct static_offsets<
    method<YOMM2_SYMBOL(meet), int(virtual_ptr<Animal>, virtual_ptr<Animal>)>> {
    static constexpr size_t slots[] = {1, 2};
    static constexpr size_t strides[] = {3};
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

register_classes(Animal, Dog, Cat);

declare_method(int, kick, (virtual_ptr<Animal>));

define_method(int, kick, (virtual_ptr<Dog> dog)) {
    return 0;
}

declare_method(int, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>));

define_method(int, meet, (virtual_ptr<Animal> a, virtual_ptr<Animal> b)) {
    return 1;
}

define_method(int, meet, (virtual_ptr<Dog> a, virtual_ptr<Dog> b)) {
    return 2;
}

define_method(int, meet, (virtual_ptr<Cat> a, virtual_ptr<Cat> b)) {
    return 3;
}

using namespace yorel::yomm2;

static_assert(detail::has_static_offsets<
              method<YOMM2_SYMBOL(kick), int(virtual_ptr<Animal>)>>::value);

int call_kick(virtual_ptr<Animal> animal) {
    return 1 + kick(animal);
}

static_assert(detail::has_static_offsets<method<
                  YOMM2_SYMBOL(meet),
                  int(virtual_ptr<Animal>, virtual_ptr<Animal>)>>::value);

int call_meet(virtual_ptr<Animal> a, virtual_ptr<Animal> b) {
    return 1 + meet(a, b);
}

void initialize() {
    default_policy::static_vptr<Dog> = nullptr;
}

int main() {
    update();

    Dog snoopy;
    Cat felix;
    std::cout << call_kick(snoopy) << "\n";
    std::cout << call_meet(snoopy, snoopy) << "\n";

    default_policy::trace_enabled = true;
    detail::runtime<default_policy> rt;
    rt.compile();
    rt.generate(std::cout);

    // initialize();
    // std::cout << call_kick(snoopy) << "\n";

    return 0;
}
