// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

// clang-format off

struct Dog {
    virtual ~Dog() {
    }
};

register_classes(Dog);

declare_method(const char*, kick, (virtual_<Dog&>));

define_method(const char*, kick, (Dog&)) {
    return "bark";
}

auto call_kick(Dog& obj) {
    return kick(obj);
}

/*
movq	context+24(%rip), %r8
movq	context+32(%rip), %rdx
movb	context+40(%rip), %cl
movslq	method<kick, char const* (virtual_<Dog&>)>::fn+96(%rip), %rsi
movq	(%rdi), %rax
imulq	-8(%rax), %rdx
shrq	%cl, %rdx
movq	(%r8,%rdx,8), %rax
jmpq	*(%rax,%rsi,8)                  # TAILCALL
*/

using const_char_ptr = const char*;

auto call_kick_manual(Dog& obj) {
    using namespace yorel::yomm2;
    const auto& context = policy::global_context::context;
    return ((const char*(*)(Dog&))(
        context.hash_table[
            std::uintptr_t(&typeid(obj)) * context.hash.mult >> context.hash.shift
        ]
        .pw[method<YOMM2_SYMBOL(kick), const_char_ptr(virtual_<Dog&>)>::fn.slots_strides.i]
        .pw))(obj);
}

BOOST_AUTO_TEST_CASE(test_manual_call) {
    yorel::yomm2::update_methods();
    Dog o;
    BOOST_TEST(call_kick_manual(o) == std::string("bark"));
}

>>>>>>> 52ebdbc (fix error handlers)
