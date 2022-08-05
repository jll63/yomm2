// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>

#include <iostream>

struct Object {
    virtual ~Object() {
    }
};

register_classes(Object);

declare_method(void, foo, (virtual_<Object&>));

define_method(void, foo, (Object&)) {
    std::cout << "ok\n";
}

using namespace yorel::yomm2;

void call_foo(Object& obj) {
    foo(obj);
}

void call_foo_manual(Object& obj) {
    const auto& context = policy::global_context::context;
    ((void (*)(Object&))(
        context
            .hash_table
                [std::uintptr_t(&typeid(obj)) * context.hash.mult >>
                 context.hash.shift]
            .pw[method<YOMM2_SYMBOL(foo), void(virtual_<Object&>)>::fn
                    .slots_strides.i]
            .pw))(obj);
}

int main() {
    yorel::yomm2::update_methods();
    Object o;
    call_foo(o);
    call_foo_manual(o);
    return 0;
}
