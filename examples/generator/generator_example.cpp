#include "animals.hpp"
#include <yorel/yomm2/generator.hpp>

#include <iomanip>

// https://godbolt.org/z/rf1bjb544

void call_vf(Animal& a) {
    // yardstick
    a.kick();
	// mov	rax, qword ptr [rdi]
	// jmp	qword ptr [rax + 16]            # TAILCALL

}

void call_kick(virtual_ptr<Animal> animal) {
    // using dynamic offsets
    kick(animal);
	// mov	rax, qword ptr [rip + method<kick, void (virtual_ptr<Animal>)>::slots_strides]
	// mov	rax, qword ptr [rsi + 8*rax]
	// jmp	rax                             # TAILCALL

}

void call_pet(virtual_ptr<Animal> animal) {
    // using static offsets
    pet(animal);
	// mov	rax, qword ptr [rsi + 32]
	// jmp	rax                             # TAILCALL

}

void call_meet(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
    // using dynamic offsets
    meet(a1, a2);
	// mov	rax, qword ptr [rip + method<meet, void (virtual_ptr<Animal>, virtual_ptr<Animal>)>::slots_strides]
	// mov	rax, qword ptr [rsi + 8*rax]
	// mov	r8, qword ptr [rip + method<meet, void (virtual_ptr<Animal>, virtual_ptr<Animal>)>::slots_strides+8]
	// mov	r8, qword ptr [rcx + 8*r8]
	// imul	r8, qword ptr [rip + method<meet, void (virtual_ptr<Animal>, virtual_ptr<Animal>)>::slots_strides+16]
	// mov	rax, qword ptr [rax + 8*r8]
	// jmp	rax                             # TAILCALL
}

void call_mate(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
    // using static offsets
    mate(a1, a2);
	// mov	rax, qword ptr [rsi]
	// mov	r8, qword ptr [rcx + 8]
	// lea	r8, [r8 + 2*r8]
	// mov	rax, qword ptr [rax + 8*r8]
	// jmp	rax                             # TAILCALL
}

int main() {
    using namespace yorel::yomm2;

    #include "tables.hpp"

    Cat felix;
    auto cat = virtual_ptr<Cat>::final(felix);
    Dog snoopy;
    virtual_ptr dog = virtual_ptr<Dog>::final(snoopy);

    // // our yardstick: an ordinary virtual function call
    // felix.kick();

    call_pet(dog);
    call_kick(cat);
    call_meet(cat, cat);
    call_mate(dog, dog);

    return 0;
}
