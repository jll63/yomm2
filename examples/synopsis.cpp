// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

// NOTE: No actual animals were hurt while designing, coding, compiling and
// running this example.

// =============================================================================
// Define a few polymorphic classes...

class Animal {
  public:
    virtual ~Animal() {
    }
};

class Dog : public Animal {};
class Bulldog : public Dog {};
class Cat : public Animal {};
class Dolphin : public Animal {};

// =============================================================================
// Add behavior to existing classes, without modifying them.

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

// Classes must be registered:
register_classes(Animal, Dog, Cat, Dolphin);

// ...but it does not have to be in one call to 'register_classes', as long as
// inheritance relationships can be deduced. This allows *adding* classes to an
// existing collection of classes.
register_classes(Dog, Bulldog);

// Define a uni-method, i.e. a method with a single virtual argument. This is in
// essence a virtual function implemented as a free function.
struct kick_method;
using YoMm2_gS_2 = ::yorel::yomm2::detail::method_macro_aux<
    kick_method, void(virtual_<Animal&>, std::ostream&),
    ::yorel::yomm2::detail::types<>>::type;
auto kick_method_guide(
    ::yorel::yomm2::detail::remove_virtual<virtual_<Animal&>> a0,
    ::yorel::yomm2::detail::remove_virtual<std::ostream&> a1) -> YoMm2_gS_2;
inline __attribute__((__always_inline__)) void kick(
    ::yorel::yomm2::detail::remove_virtual<virtual_<Animal&>> a0,
    ::yorel::yomm2::detail::remove_virtual<std::ostream&> a1) {
    return YoMm2_gS_2::fn(
        std::forward<::yorel::yomm2::detail::remove_virtual<virtual_<Animal&>>>(
            a0),
        std::forward<::yorel::yomm2::detail::remove_virtual<std::ostream&>>(
            a1));
}
template<typename MethodSignature, typename OverriderSignature>
struct kick_overriders;
template<
    typename OverriderReturnType,
    typename... OverriderParameters>
struct kick_overriders<
    kick_method(virtual_<Animal&>, std::ostream&),
    OverriderReturnType(OverriderParameters...)> {
    static typename YoMm2_gS_2::next_type next;
    static OverriderReturnType fn(OverriderParameters...);
};

// Implement 'kick' for dogs.
template<typename...>
struct YoMm2_gS_3;
template<typename... Parameters>
struct YoMm2_gS_3<void(Parameters...)> {
    using type = decltype(kick_method_guide(std::declval<Parameters>()...));
};
template<>
auto kick_overriders<
    kick_method(virtual_<Animal&>, std::ostream&),
    void(Dog& dog, std::ostream& os)>::fn(Dog& dog, std::ostream& os) -> void;
static YoMm2_gS_3<void(Dog& dog, std::ostream& os)>::type::override<
    kick_overriders<
        YoMm2_gS_3<void(Dog& dog, std::ostream& os)>::type,
        void(Dog& dog, std::ostream& os)>>
    YoMm2_gS_4;
template<>
auto kick_overriders<
    YoMm2_gS_3<void(Dog& dog, std::ostream& os)>::type::this_type,
    void(Dog& dog, std::ostream& os)>::fn(Dog& dog, std::ostream& os) -> void {
    os << "bark";
}

// Implement 'kick' for bulldogs. They behave like Dogs, but, in addition, they
// fight back.
define_method(void, kick, (Bulldog& dog, std::ostream& os)) {
    next(dog, os); // calls "base" method, i.e. definition for Dog
    os << " and bite";
}

// A multi-method with two virtual arguments...
declare_method(
    void, meet, (virtual_<Animal&>, virtual_<Animal&>, std::ostream&));

// 'meet' catch-all implementation.
define_method(void, meet, (Animal&, Animal&, std::ostream& os)) {
    os << "ignore";
}

// Add definitions for specific pairs of animals.
define_method(void, meet, (Dog& dog1, Dog& dog2, std::ostream& os)) {
    os << "wag tail";
}

define_method(void, meet, (Dog& dog, Cat& cat, std::ostream& os)) {
    os << "chase";
}

define_method(void, meet, (Cat& cat, Dog& dog, std::ostream& os)) {
    os << "run";
}

// =============================================================================
// main

#include <iostream>
#include <memory>
#include <string>

int main() {
    // Initialise method dispatch tables.
    yorel::yomm2::initialize();

    // Create a few objects.
    // Note that the actual classes are type-erased to base class Animal!
    std::unique_ptr<Animal>
        hector = std::make_unique<Bulldog>(),
        snoopy = std::make_unique<Dog>(),
        sylvester = std::make_unique<Cat>(),
        flipper = std::make_unique<Dolphin>();

    // Call 'kick'.
    std::cout << "kick snoopy: ";
    kick(*snoopy, std::cout); // bark
    std::cout << "\n";

    std::cout << "kick hector: ";
    kick(*hector, std::cout); // bark and bite
    std::cout << "\n";

    // Call 'meet'.
    std::cout << "hector meets sylvester: ";
    meet(*hector, *sylvester, std::cout); // chase
    std::cout << "\n";

    std::cout << "sylvester meets hector: ";
    meet(*sylvester, *hector, std::cout); // run
    std::cout << "\n";

    std::cout << "hector meets snoopy: ";
    meet(*hector, *snoopy, std::cout); // wag tail
    std::cout << "\n";

    std::cout << "hector meets flipper: ";
    meet(*hector, *flipper, std::cout); // ignore
    std::cout << "\n";
}

// Let's look at the code generated by clang++-14 for method call.

void call_kick(Animal& a, std::ostream& os) {
    kick(a, os);

    // Instructions in the same paragraph are independent, thus they can be
    // executed in parallel.

	// mov	rax, qword ptr [rdi]                ; read vptr
	// mov	rdx, qword ptr [rip + global+24]    ; M hash factor (multiply)

	// imul	rdx, qword ptr [rax - 8]            ; multiply vptr[-1] (&typeid(a)) by M
	// mov	cl, byte ptr [rip + global+32]      ; S hash factor (shift)

	// shr	rdx, cl                             ; shift by S: this is the position of
    //                                          ; the method table for the dynamic class of 'a'
    //                                          ; in the global hash table
	// mov	rax, qword ptr [rip + global+40]    ; address of global hash table

	// mov	rax, qword ptr [rax + 8*rdx]        ; method table for the class
	// mov	rcx, qword ptr [rip + method+96]    ; offset of the 'kick' in method table

	// mov	rax, qword ptr [rax + 8*rcx]        ; read function pointer at offset

	// jmp	rax                                 ; tail call
}

void call_meet(Animal& a, Animal& b, std::ostream& os) {
    meet(a, b, os);

    // Instructions in the same paragraph are independent, thus they can be
    // executed in parallel.

	// mov	r8, qword ptr [rdi]                 ; vptr of 'a'
	// mov	r9, qword ptr [rip + global+24]     ; M hash factor (multiply)
	// mov	cl, byte ptr [rip + global+32]      ; S hash factor (shift)

	// mov	r10, qword ptr [r8 - 8]             ; read a.vptr[-1] (&a)

	// imul	r10, r9                             ; multiply by M

	// shr	r10, cl                             ; index of method table for 'a'
	// mov	r8, qword ptr [rip + global+40]     ; address of global hash table
	// mov	rax, qword ptr [rsi]                ; read vptr of 'b'
	// imul	r9, qword ptr [rax - 8]             ; multiply b.vptr[-1] (&typeid(b)) by M

	// mov	rax, qword ptr [r8 + 8*r10]         ; method table for 'a'
	// shr	r9, cl                              ; index of method table for 'b'

	// mov	rcx, qword ptr [rip + method+96]    ; offset of 'meet' in method table for 'a'

	// mov	r10, qword ptr [rax + 8*rcx]        ; pointer to row for 'a' in dispatch table
	// mov	rcx, qword ptr [r8 + 8*r9]          ; method table for 'b'
	// mov	rax, qword ptr [rip + method+104]   ; offset of 'meet' in method table for 'b'

	// mov	rax, qword ptr [rcx + 8*rax]        ; column of 'b' in dispatch table

	// imul	rax, qword ptr [rip + method+112]   ; multiply by # of lines in dispatch table

	// mov	rax, qword ptr [r10 + 8*rax]        ; pointer to function, at dispatch[a][b]

	// jmp	rax                                 ; tail call
}
