// Copyright (c) 2018-2022 Jean-Louis Leroy
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
#include <yorel/yomm2/runtime.hpp>

namespace yorel {
namespace yomm2 {

namespace policy {

template class yOMM2_API_msc generic_domain<debug_shared>;
template class yOMM2_API_msc generic_error_handler<debug_shared>;
template class yOMM2_API_msc backward_compatible_error_handler<debug_shared>;
template class yOMM2_API_msc fast_projection<debug_shared>;
template class yOMM2_API_msc checked_fast_projection<debug_shared>;
template class yOMM2_API_msc generic_output<debug_shared>;
template class yOMM2_API_msc generic_policy<
    debug_shared, generic_domain<debug_shared>, std_rtti,
    checked_fast_projection<debug_shared>, generic_output<debug_shared>,
    backward_compatible_error_handler<debug_shared>>;

} // namespace policy

template void yOMM2_API_gcc yOMM2_API_msc update<policy::debug_shared>();

yOMM2_API_gcc yOMM2_API_msc void update() {
    update<policy::debug_shared>();
}

yOMM2_API_gcc yOMM2_API_msc error_handler_type set_error_handler(error_handler_type handler) {
    auto prev = default_policy::error;
    default_policy::error = handler;
    return prev;
}

yOMM2_API_gcc yOMM2_API_msc method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler) {
    auto prev = default_policy::call_error;
    default_policy::call_error = handler;
    return prev;
}

} // namespace yomm2
} // namespace yorel
