// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_HPP
#define YOREL_YOMM2_HPP

#include <algorithm> // IWYU pragma: keep
#include <iostream>
#include <iterator> // IWYU pragma: keep
#include <type_traits>
#include <typeinfo>
#include <unordered_set>
#include <utility>

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/macros.hpp>

#define register_classes YOMM2_CLASSES
#define declare_method YOMM2_METHOD
#define declare_static_method YOMM2_STATIC_METHOD
#define define_method YOMM2_OVERRIDE
#define define_method_in YOMM2_OVERRIDE_IN
#define define_method_inline YOMM2_OVERRIDE_INLINE
#define method_class YOMM2_METHOD_CLASS

using yorel::yomm2::virtual_;
using yorel::yomm2::virtual_ptr;

#endif
