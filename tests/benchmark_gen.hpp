#define POLICY default_policy

#define INHERITANCE
#define PREFIX normal
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#undef PREFIX

#define INHERITANCE virtual
#define PREFIX virtual
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#undef PREFIX

#undef POLICY
#define POLICY hash_in_gv_policy

#define INHERITANCE
#define PREFIX hash_in_gv
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#undef PREFIX
