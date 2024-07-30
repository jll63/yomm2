#ifndef ANIMALS_HPP
#define ANIMALS_HPP

#include <iosfwd>

#include <yorel/yomm2/policy.hpp>

// Override the default policy with a minimal policy:
//  * no external vptr table, because we will only use 'final'
//  * therefore, no need for a hash function
// We use only the 'std_rtti' facet, which is needed by the generator. If this
// was not acceptable, we could build the 'generate' program with RTTI enabled,
// and the application without RTTI. This would require building the 'animals'
// classes twice, but the debug build could be used for the generation.
struct animals_policy : yorel::yomm2::policy::basic_policy<
                            animals_policy, yorel::yomm2::policy::std_rtti> {};

#define YOMM2_DEFAULT_POLICY animals_policy

#include <yorel/yomm2/keywords.hpp>

using yorel::yomm2::virtual_ptr;

struct Animal {
    virtual ~Animal();
    virtual void kick() = 0;
};

struct Dog : Animal {
    virtual void kick() {
    }
};

struct Cat : Animal {
    virtual void kick() {
    }
};

declare_method(void, kick, (virtual_ptr<Animal>));
declare_method(void, pet, (virtual_ptr<Animal>));
declare_method(
    void, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>));
declare_method(
    void, mate, (virtual_ptr<Animal>, virtual_ptr<Animal>));

#if __has_include("slots.hpp")
#include "slots.hpp"
#endif

#endif
