#include "animals.hpp"
#include <yorel/yomm2/generator.hpp>

#include <iomanip>

// https://godbolt.org/z/rf1bjb544

void call_vf(Animal& a) {
    // yardstick
    a.kick();
}

void call_kick(virtual_ptr<Animal> animal) {
    // using dynamic offsets
    kick(animal);
}

void call_pet(virtual_ptr<Animal> animal) {
    // using static offsets
    pet(animal);
}

void call_meet(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
    // using dynamic offsets
    meet(a1, a2);
}

void call_mate(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
    // using static offsets
    mate(a1, a2);
}

#include "tables.hpp"

template<class Policy>
auto collect_slots() {
    std::vector<std::uintptr_t> slots_strides;

    for (auto& method : Policy::methods) {
        std::copy_n(
            method.slots_strides_ptr, method.arity(),
            std::back_inserter(slots_strides));
    }

    return slots_strides;
}

int main() {
    using namespace yorel::yomm2;

    #ifdef CHECK

    auto compiler = update();

    std::vector<std::uintptr_t> expected_slots_strides =
        collect_slots<default_policy>();
    auto expected_dispatch_data = default_policy::dispatch_data;

    for (auto& cls : default_policy::classes) {
        *cls.static_vptr = nullptr;
    }

    #endif

    auto unpacked = generated();

    #ifdef CHECK

    auto actual_slots_strides = collect_slots<default_policy>();

    if (actual_slots_strides.size() != expected_slots_strides.size()) {
        std::cerr << "slot count mismatch: " << actual_slots_strides.size()
                  << " != " << expected_slots_strides.size() << "\n";
    }

#undef min
    auto n =
        std::min(actual_slots_strides.size(), expected_slots_strides.size());

    for (size_t i = 0; i < n; ++i) {
        if (actual_slots_strides[i] != expected_slots_strides[i]) {
            std::cerr << "slot " << i << " mismatch: " << std::hex
                      << actual_slots_strides[i]
                      << " != " << expected_slots_strides[i] << "\n";
        }
    }

    auto expected_size = expected_dispatch_data.size();
    auto actual_size = default_policy::dispatch_data.size();
    n = std::min(
        default_policy::dispatch_data.size(), expected_dispatch_data.size());

    if (actual_size != expected_size) {
        std::cerr << "dispatch_data mismatch: " << actual_size
                  << " != " << expected_size << "\n";
    }

    // for (size_t i = 0; i < n; ++i) {
    //     if (default_policy::dispatch_data[i] != unpacked[i]) {
    //         std::cerr << "dispatch_data " << i << " : " << std::hex
    //                   << default_policy::dispatch_data[i]
    //                   << " != " << unpacked[i] << "\n";
    //     }
    // }

    #endif

    Animal&& felix = Cat();
    virtual_ptr cat = felix;
    Animal&& snoopy = Dog();
    virtual_ptr dog = snoopy;

    // // our yardstick: an ordinary virtual function call
    // felix.kick();

    kick(cat);
    pet(dog);
    meet(cat, cat);
    mate(dog, dog);
    call_mate(dog, dog);

    return 0;
}
