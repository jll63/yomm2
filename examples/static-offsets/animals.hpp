#ifndef ANIMALS_HPP
#define ANIMALS_HPP

#include <iosfwd>
#include <yorel/yomm2/keywords.hpp>

using yorel::yomm2::virtual_ptr;

#if __has_include("animals-yomm2-offsets.hpp")
#include "animals-yomm2-offsets.hpp"
#endif

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

declare_method(void, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>));
declare_method(void, kick, (virtual_ptr<Animal>));

#endif
