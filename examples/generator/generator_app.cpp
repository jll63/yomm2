#include "animals.hpp"
#include <yorel/yomm2/generator.hpp>

#include <iomanip>

void call_vf(Animal& a) {
    // yardstick
    a.kick();
    // mov	rax, qword ptr [rdi]
    // jmp	qword ptr [rax + 16]            # TAILCALL
}

void call_kick(virtual_ptr<Animal> animal) {
    kick(animal);
    // mov	rax, qword ptr [rsi + 16]
    // jmp	rax                             # TAILCALL
}

void call_meet(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
    meet(a1, a2);
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

    call_kick(cat);
    call_meet(dog, cat);

    return 0;
}
