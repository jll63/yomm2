// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(YOMM2_SHARED)
#error "This should be compiled only for a shared library build."
#endif

#if defined(_MSC_VER)
#define yOMM2_API_msc __declspec(dllexport)
#define yOMM2_DLL
#else
#define yOMM2_API_gcc __attribute__((__visibility__("default")))
#endif

#include <yorel/yomm2/core.hpp>

namespace yorel {
namespace yomm2 {

namespace policy {

//std::vector<type_id> checked_perfect_hash<debug_shared>::control;

template class yOMM2_API_msc basic_domain<debug_shared>;
template class yOMM2_API_msc vptr_vector<debug_shared>;
template class yOMM2_API_msc basic_indirect_vptr<debug_shared>;
template class yOMM2_API_msc fast_perfect_hash<debug_shared>;
template class yOMM2_API_msc checked_perfect_hash<debug_shared>;
template class yOMM2_API_msc basic_error_output<debug_shared>;
template class yOMM2_API_msc basic_trace_output<debug_shared>;
template class yOMM2_API_msc vectored_error<debug_shared>;
template class yOMM2_API_msc basic_policy<
    debug_shared, std_rtti, checked_perfect_hash<debug_shared>,
    basic_error_output<debug_shared>, basic_trace_output<debug_shared>>;

} // namespace policy

template auto yOMM2_API_gcc yOMM2_API_msc update<policy::debug_shared>()
    -> detail::compiler<policy::debug_shared>;

yOMM2_API_gcc yOMM2_API_msc auto update()
    -> detail::compiler<policy::debug_shared> {
    return update<policy::debug_shared>();
}

yOMM2_API_gcc yOMM2_API_msc error_handler_type
set_error_handler(error_handler_type handler) {
    auto prev = default_policy::error;
    default_policy::error = handler;
    return prev;
}


} // namespace yomm2
} // namespace yorel
