#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>
#include <boost/utility/identity_type.hpp>

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/compiler.hpp>

using namespace yorel::yomm2;

namespace type_hash {

struct Animal {
    const char* name;

    Animal(const char* name, const char* type) : name(name), type(type) {
    }

    static constexpr const char* static_type = "Animal";
    const char* type;
};

struct Dog : Animal {
    Dog(const char* name, const char* type = static_type) : Animal(name, type) {
    }
    static constexpr const char* static_type = "Dog";
};

struct Cat : Animal {
    Cat(const char* name, const char* type = static_type) : Animal(name, type) {
    }
    static constexpr const char* static_type = "Cat";
};

struct custom_rtti : policy::rtti {
    template<typename T>
    static auto static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return reinterpret_cast<type_id>(T::static_type);
        } else {
            return 0;
        }
    }

    template<typename T>
    static auto dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return reinterpret_cast<type_id>(obj.type);
        } else {
            return 0;
        }
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << (type == 0 ? "?" : reinterpret_cast<const char*>(type));
    }

    static auto type_index(type_id type) {
        return std::string_view(
            (type == 0 ? "?" : reinterpret_cast<const char*>(type)));
    }
};

struct test_policy : policy::default_static::rebind<test_policy>::replace<
                         policy::rtti, custom_rtti> {};

register_classes(Animal, Dog, Cat, test_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), test_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}

BOOST_AUTO_TEST_CASE(custom_rtti_simple_projection) {
    update<test_policy>();

    Animal &&a = Dog("Snoopy"), &&b = Cat("Sylvester");

    {
        std::stringstream os;
        kick(a, os);
        BOOST_TEST(os.str() == "Snoopy barks.");
    }
    {
        std::stringstream os;
        kick(b, os);
        BOOST_TEST(os.str() == "Sylvester hisses.");
    }
}
} // namespace type_hash

namespace no_projection {

struct Animal {
    const char* name;

    Animal(const char* name, size_t type) : name(name), type(type) {
    }

    static constexpr size_t static_type = 0;
    size_t type;
};

struct Dog : Animal {
    Dog(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static constexpr size_t static_type = 1;
};

struct Cat : Animal {
    Cat(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static constexpr size_t static_type = 2;
};

struct custom_rtti : policy::rtti {
    template<typename T>
    static auto static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return T::static_type;
        } else {
            return 666;
        }
    }

    template<typename T>
    static auto dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return obj.type;
        } else {
            return 666;
        }
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        static const char* name[] = {"Animal", "Dog", "Cat"};
        stream << (type == 666 ? "?" : name[type]);
    }

    static auto type_index(type_id type) {
        return type;
    }
};

struct test_policy : policy::default_static::rebind<test_policy>::replace<
                         policy::rtti, custom_rtti>::remove<policy::type_hash> {
};

register_classes(Animal, Dog, Cat, test_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), test_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}

void call_kick(Animal& a, std::ostream& os) {
    return kick(a, os);
}
// mov     rax, qword ptr [rdi + 8]
// mov     rcx, qword ptr [rip + basic_domain<test_policy>::context+24]
// mov     rax, qword ptr [rcx + 8*rax]
// mov     rcx, qword ptr [rip + method<test_policy, kick, void
// (virtual_<Animal&>, basic_ostream<char, char_traits<char> >&)>::fn+80] mov
// rax, qword ptr [rax + 8*rcx] jmp     rax                             #
// TAILCALL

BOOST_AUTO_TEST_CASE(custom_rtti_simple) {
    BOOST_TEST(Animal::static_type == 0);
    BOOST_TEST(Dog::static_type == 1);
    BOOST_TEST(Cat::static_type == 2);
    update<test_policy>();

    Animal &&a = Dog("Snoopy"), &&b = Cat("Sylvester");

    {
        std::stringstream os;
        kick(a, os);
        BOOST_TEST(os.str() == "Snoopy barks.");
    }
    {
        std::stringstream os;
        kick(b, os);
        BOOST_TEST(os.str() == "Sylvester hisses.");
    }
}

namespace using_vptr {

template<class C>
using vptr = virtual_ptr<C, test_policy>;

declare_method(void, kick, (vptr<Animal>, std::ostream&), test_policy);

define_method(void, kick, (vptr<Dog> dog, std::ostream& os)) {
    os << dog->name << " barks.";
}

define_method(void, kick, (vptr<Cat> cat, std::ostream& os)) {
    os << cat->name << " hisses.";
}

void call_kick(vptr<Animal> a, std::ostream& os) {
    // mov     rax, qword ptr [rip + method<test_policy, kick, ...>::fn+80]
    // mov     rax, qword ptr [rsi + 8*rax]
    // jmp     rax                             # TAILCALL
    return kick(a, os);
}

} // namespace using_vptr
} // namespace no_projection

namespace virtual_base {

struct Animal {
    const char* name;

    Animal(const char* name, size_t type) : name(name), type(type) {
    }

    virtual void* cast_aux(size_t type) {
        return type == static_type ? this : nullptr;
    }

    static constexpr size_t static_type = 0;
    size_t type;
};

template<typename Derived, typename Base>
Derived custom_dynamic_cast(Base& obj) {
    using derived_type = std::remove_cv_t<std::remove_reference_t<Derived>>;
    return *reinterpret_cast<derived_type*>(
        const_cast<std::remove_cv_t<Base>&>(obj).cast_aux(
            derived_type::static_type));
}

struct Dog : virtual Animal {
    Dog(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    void* cast_aux(size_t type) override {
        return type == static_type ? this : Animal::cast_aux(type);
    }

    static constexpr size_t static_type = 1;
};

struct Cat : virtual Animal {
    Cat(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    void* cast_aux(size_t type) override {
        return type == static_type ? this : Animal::cast_aux(type);
    }

    static constexpr size_t static_type = 2;
};

struct custom_rtti : policy::rtti {
    template<typename T>
    static auto static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return T::static_type;
        } else {
            return 666;
        }
    }

    template<typename T>
    static auto dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return obj.type;
        } else {
            return 666;
        }
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        static const char* name[] = {"Animal", "Dog", "Cat"};
        stream << (type == 666 ? "?" : name[type]);
    }

    static auto type_index(type_id type) {
        return type;
    }

    template<typename Derived, typename Base>
    static Derived dynamic_cast_ref(Base&& obj) {
        return custom_dynamic_cast<Derived>(obj);
    }
};

struct test_policy : policy::default_static::rebind<test_policy>::replace<
                         policy::rtti, custom_rtti>::remove<policy::type_hash> {
};

register_classes(Animal, Dog, Cat, test_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), test_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}

void call_kick(Animal& a, std::ostream& os) {
    return kick(a, os);
}
// mov     rax, qword ptr [rdi + 8]
// mov     rcx, qword ptr [rip + basic_domain<test_policy>::context+24]
// mov     rax, qword ptr [rcx + 8*rax]
// mov     rcx, qword ptr [rip + method<test_policy, kick, void
// (virtual_<Animal&>, basic_ostream<char, char_traits<char> >&)>::fn+80] mov
// rax, qword ptr [rax + 8*rcx] jmp     rax                             #
// TAILCALL

BOOST_AUTO_TEST_CASE(virtual_base) {
    BOOST_TEST(Animal::static_type == 0);
    BOOST_TEST(Dog::static_type == 1);
    BOOST_TEST(Cat::static_type == 2);
    update<test_policy>();

    Animal &&a = Dog("Snoopy"), &&b = Cat("Sylvester");

    {
        std::stringstream os;
        kick(a, os);
        BOOST_TEST(os.str() == "Snoopy barks.");
    }
    {
        std::stringstream os;
        kick(b, os);
        BOOST_TEST(os.str() == "Sylvester hisses.");
    }
}

namespace using_vptr {

template<class C>
using vptr = virtual_ptr<C, test_policy>;

declare_method(void, kick, (vptr<Animal>, std::ostream&), test_policy);

define_method(void, kick, (vptr<Dog> dog, std::ostream& os)) {
    os << dog->name << " barks.";
}

define_method(void, kick, (vptr<Cat> cat, std::ostream& os)) {
    os << cat->name << " hisses.";
}

void call_kick(vptr<Animal> a, std::ostream& os) {
    // mov     rax, qword ptr [rip + method<test_policy, kick, ...>::fn+80]
    // mov     rax, qword ptr [rsi + 8*rax]
    // jmp     rax                             # TAILCALL
    return kick(a, os);
}

} // namespace using_vptr
} // namespace virtual_base

namespace defered_type_id {

struct Animal {
    const char* name;

    Animal(const char* name, size_t type) : name(name), type(type) {
    }

    static size_t last_type_id;
    static size_t static_type;
    size_t type;
};

size_t Animal::last_type_id;
size_t Animal::static_type = ++Animal::last_type_id;

struct Dog : Animal {
    Dog(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static size_t static_type;
};

size_t Dog::static_type = ++Animal::last_type_id;

struct Cat : Animal {
    Cat(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static size_t static_type;
};

size_t Cat::static_type = ++Animal::last_type_id;

struct custom_rtti : policy::deferred_static_rtti {
    template<typename T>
    static auto static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return T::static_type;
        } else {
            static type_id invalid = 0;
            return invalid;
        }
    }

    template<typename T>
    static auto dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return obj.type;
        } else {
            return 666;
        }
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        static const char* name[] = {"Animal", "Dog", "Cat"};
        stream << (type == 0 ? "?" : name[type]);
    }

    static auto type_index(type_id type) {
        return type;
    }
};

struct test_policy : policy::default_static::rebind<test_policy>::replace<
                         policy::rtti, custom_rtti>::remove<policy::type_hash> {
};

register_classes(Animal, Dog, Cat, test_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), test_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}

BOOST_AUTO_TEST_CASE(custom_rtti_deferred) {
    update<test_policy>();

    Animal &&a = Dog("Snoopy"), &&b = Cat("Sylvester");

    {
        std::stringstream os;
        kick(a, os);
        BOOST_TEST(os.str() == "Snoopy barks.");
    }
    {
        std::stringstream os;
        kick(b, os);
        BOOST_TEST(os.str() == "Sylvester hisses.");
    }
}

} // namespace defered_type_id
