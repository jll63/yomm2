// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define INHERITANCE
#define PREFIX normal
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#undef PREFIX

#define INHERITANCE virtual
#define PREFIX virtual
#include "benchmark_matrices.hpp"
