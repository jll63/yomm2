// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#if defined(YOMM2_SHARED)
    #if defined(_WIN32)
        #define yOMM2_API __declspec(dllexport)
    #else
        #define yOMM2_API __attribute__((__visibility__("default")))
    #endif
#else
    #error "This should be compiled only for a shared library build."
#endif

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/runtime.hpp>

namespace yorel {
namespace yomm2 {

namespace policy {
yOMM2_API context abstract_shared::context;
yOMM2_API catalog abstract_shared::catalog;
yOMM2_API detail::stdostream abstract_shared::trace;
} // namespace policy

template void update<policy::abstract_shared>();

yOMM2_API void update() {
    update<policy::abstract_shared>();
}

} // namespace yomm2
} // namespace yorel
