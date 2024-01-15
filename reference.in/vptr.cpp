#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

// #ifdef YOMM2_MD

// <sub>/ ->home / ->reference </sub>

// entry: yorel::yomm2::policy::vptr
// entry: yorel::yomm2::policy::external_vptr
// entry: yorel::yomm2::policy::external_vptr_vector
// entry: yorel::yomm2::policy::external_vptr_map
// headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

// ---
// ```
// struct vptr;

// struct external_vptr;

// template<class Policy>
// struct external_vptr_vector;

// template<class Policy>
// struct external_vptr_map;
// ```
// ---
// A `vptr` facet is provides a static function that returns a pointer to the
// dispatch data for a virtual argument's dynamic class.

// YOMM2 implements method dispatch in a way similar to native virtual function
// dispatch: for each virtual argument, fetch a pointer to the dispatch data
// (the v-table), and use it to select a pointer to a function. Method v-tables
// contain pointers to functions for unary methods, and, for multi-methods,
// pointers to, and coordinates in, a multi-dimensional table of pointers to
// functions.

// The `vptr` facet is used during method call to fetch the vptr for virtual
// arguments corresponding to the `virtual_` parameters in the method
// declaration. It is also used by the constructor of `virtual_ptr` to obtain a
// vptr on the basis of an object's dynamic type.

// `virtual_ptr::final`, and the related convenience functions, assume that the
// static and dynamic types of their argument are the same. The vptr is obtained
// statically from the policy's `static_vptr<Class>` member. It is conceivable
// to organize an entire program around the "final" constructs; thus, the `vptr`
// facet is optional.

// `external_vptr` is a sub-category of `facet`. If present, the runtime calls
// its static functions to allow it to initialize its data structures.

// #endif

#include <iostream>
#include <vector>

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace policy;

struct vptr_page : virtual vptr {
    static std::vector<const std::uintptr_t*> vptrs;

    template<typename ForwardIterator>
    static void register_vptrs(ForwardIterator first, ForwardIterator last) {
    }

    template<class Class>
    static auto vptr(const Class& arg) {
        auto page = reinterpret_cast<std::uintptr_t>(&arg) & ~1023;
        return *reinterpret_cast<std::uintptr_t**>(page);
    }
};

struct number_policy : default_policy::replace<vptr, vptr_page> {};

template<class T>
class naive_page_allocator {
  private:
    static void* top;
    static size_t space;

    template<typename U>
    static auto alloc(size_t n) -> U* {
        if (auto result = std::align(alignof(U), n * sizeof(U), top, space)) {
            top = reinterpret_cast<char*>(top) + n * sizeof(U);
            space -= n * sizeof(U);
            return reinterpret_cast<U*>(result);
        }

        throw std::bad_alloc();
    }

  public:
    static void intialize() {
        *alloc<std::uintptr_t**>(1) = number_policy::static_vptr<T>;
    }

    typedef T value_type;

    naive_page_allocator() {
    }

    template<class U>
    constexpr naive_page_allocator(const naive_page_allocator<U>&) noexcept {
    }

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        return alloc<T>(n);
    }

    void deallocate(T* p, std::size_t n) noexcept {
    }
};

template<class T>
void* naive_page_allocator<T>::top;

template<class T>
size_t naive_page_allocator<T>::space;

struct Number {};

template<class Class>
struct PageObject {
    static naive_page_allocator<Class> allocator;
    static void* operator new(size_t) {
        return allocator.allocate(1);
    }
    static void* operator new[](size_t count) {
        return allocator.allocate(count);
    }
};

template<class Class>
naive_page_allocator<Class> PageObject<Class>::allocator;

struct Integer : Number, PageObject<Integer> {
    explicit Integer(int value) : value(value) {
    }

    int value;
};

struct Rational : Number, PageObject<Rational> {
    Rational(int a, int b) : num(a), den(b) {
    }
    int num, den;
};

template<class T, class U>
bool operator==(
    const naive_page_allocator<T>&, const naive_page_allocator<U>&) {
    return true;
}

template<class T, class U>
bool operator!=(
    const naive_page_allocator<T>&, const naive_page_allocator<U>&) {
    return false;
}

// #undef YOMM2_DEFAULT_POLICY
// #define YOMM2_DEFAULT_POLICY number_policy

declare_method(
    void, add,
    (virtual_<const Number&>, virtual_<const Number&>, std::ostream&));

define_method(
    void, add, (const Integer& a, const Integer& b, std::ostream& os)) {
    os << a.value + b.value;
}

BOOST_AUTO_TEST_CASE(ref_vptr_page) {
    std::vector<int, naive_page_allocator<int>> integers = { 1, 2, 3 };

    // naive_page_allocator<Integer>::intialize();
    // const Number& i1 = *new Integer(1);
    // const Number& i2 = *new Integer(2);
}
