// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/symbols.hpp>

using namespace yorel::yomm2;

struct Dog {
    virtual ~Dog() {}
};

// Define a method and a definition, using the core API. Code below is
// equivalent to:
// register_classes(Dog);
// declare_method(const char*, kick, (virtual_<Dog&>));
// define_method(const char*, kick, (Dog&)) {
//     return "bark";
// }

use_classes<Dog> YOMM2_GENSYM;

struct kick_;
using kick = method<kick_, const char*(virtual_<Dog&>)>;

const char* bark(Dog&) {
    return "bark";
}

kick::add_function<bark> YOMM2_GENSYM;

auto call_kick(Dog& obj) {
    return kick::fn(obj);

    // corresponding machine code:
    // movq	    context+24(%rip), %r8
    // movq	    context+32(%rip), %rdx
    // movb	    context+40(%rip), %cl
    // movslq	kick::fn+96(%rip), %rsi
    // movq	    (%rdi), %rax
    // imulq	-8(%rax), %rdx
    // shrq	    %cl, %rdx
    // movq	    (%r8,%rdx,8), %rax
    // jmpq	    *(%rax,%rsi,8)                  # TAILCALL
}

auto call_kick_manual(Dog& obj) {
    const auto hash_table = policy::global_context::context.mptrs.data();
    const auto mult = policy::global_context::context.hash.mult;
    const auto shift = policy::global_context::context.hash.shift;
    const auto index = kick::fn.slots_strides[0];
    const auto vptr = *(void***) &obj; // typeid 1
	// movq	    context+24(%rip), %r8
	// movq	    context+32(%rip), %rdx
	// movb	    context+40(%rip), %cl
	// movslq	method<kick_, char const* (virtual_<Dog&>)>::fn+96(%rip), %rsi
    // movq	    (%rdi), %rax
    
    const auto h1 = mult * std::uintptr_t(vptr[-1]) /* typeid 2*/;
	// imulq	-8(%rax), %rdx

    const auto h2 = h1 >> shift;
	// shrq	    %cl, %rdx

    const auto method_table = hash_table[h2];
	// movq	    (%r8,%rdx,8), %rax

    const auto fptr = method_table[index].pw;
    return ((const char*(*)(Dog&)) fptr)(obj);
	// jmpq	    *(%rax,%rsi,8)                  # TAILCALL
}

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_manual_call) {
    yorel::yomm2::update_methods();
    Dog o;
    BOOST_TEST(call_kick_manual(o) == std::string("bark"));
}
