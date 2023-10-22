#ifndef YOREL_YOMM2_INTRUSIVE_INCLUDED
#define YOREL_YOMM2_INTRUSIVE_INCLUDED

// clang-format off

#include <yorel/yomm2/core.hpp>

namespace yorel {
namespace yomm2 {

namespace detail {

template<typename Policy>
using intrusive_mptr_type = std::conditional_t<
    Policy::use_indirect_method_pointers,
    detail::mptr_type*,
    detail::mptr_type
>;

}

template<class Class, class Policy = default_policy>
struct root {
    using YoMm2_S_mptr_policy_ = Policy;
    detail::intrusive_mptr_type<Policy> YoMm2_S_mptr_;

    root() {
        if constexpr (Policy::use_indirect_method_pointers) {
            YoMm2_S_mptr_ = &Policy::template method_table<Class>;
        } else {
            YoMm2_S_mptr_ = Policy::template method_table<Class>;
        }
    }

    auto yomm2_mptr() const {
        if constexpr (Policy::use_indirect_method_pointers) {
            return *YoMm2_S_mptr_;
        } else {
            return YoMm2_S_mptr_;
        }
    };

    void yomm2_mptr(detail::intrusive_mptr_type<Policy> mptr) {
        YoMm2_S_mptr_ = mptr;
    };
};

template<class...>
struct derived;

template<class Class>
struct derived<Class> {
    derived() {
        if constexpr (Class::YoMm2_S_mptr_policy_::use_indirect_method_pointers) {
            static_cast<Class*>(this)->yomm2_mptr(
                &Class::YoMm2_S_mptr_policy_::template method_table<Class>);
        } else {
            static_cast<Class*>(this)->yomm2_mptr(
                Class::YoMm2_S_mptr_policy_::template method_table<Class>);
        }
    }
};

template<class Class, class Base1, class... Bases>
struct derived<Class, Base1, Bases...> {
    derived() {
        if constexpr (Base1::YoMm2_S_mptr_policy_::use_indirect_method_pointers) {
            yomm2_mptr(&Base1::YoMm2_S_mptr_policy_::template method_table<
                Class>);
        } else {
            static_assert(detail::has_mptr<Base1>);
            yomm2_mptr(Base1::YoMm2_S_mptr_policy_::template method_table<
                Class>);
        }
    }

    auto yomm2_mptr() const {
        return static_cast<const Class*>(this)->Base1::yomm2_mptr();
    };

    void yomm2_mptr(detail::intrusive_mptr_type<typename Base1::YoMm2_S_mptr_policy_> mptr) {
        auto this_ = static_cast<Class*>(this);
        static_cast<Base1*>(this_)->yomm2_mptr(mptr);
        (static_cast<Bases*>(this_)->yomm2_mptr(mptr), ...);
    }
};

} // namespace yomm2
} // namespace yorel

#endif
