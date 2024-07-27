#ifndef YOREL_YOMM2_DETAIL_CLASS_INFO_HPP
#define YOREL_YOMM2_DETAIL_CLASS_INFO_HPP

namespace yorel {
namespace yomm2 {
namespace detail {

struct class_info : static_chain<class_info>::static_link {
    type_id type;
    std::uintptr_t** static_vptr;
    type_id *first_base, *last_base;
    bool is_abstract{false};

    const std::uintptr_t* vptr() const {
        return *static_vptr;
    }

    const std::uintptr_t* const* indirect_vptr() const {
        return static_vptr;
    }

    auto type_id_begin() const {
        return &type;
    }

    auto type_id_end() const {
        return &type + 1;
    }
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif // YOREL_YOMM2_DETAIL_CLASS_INFO_HPP
