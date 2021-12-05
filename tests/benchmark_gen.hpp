#define POLICY ::yorel::yomm2::policy::hash_factors_in_method
#define INHERITANCE
#define PREFIX default
#include "benchmark_matrices.hpp"
#undef INHERITANCE
#undef PREFIX

#define INHERITANCE virtual
#define PREFIX virtual
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#define INHERITANCE

#undef PREFIX
#undef POLICY

#define POLICY ::yorel::yomm2::policy::hash_factors_in_globals
#define PREFIX hash_in_globals
#include "benchmark_matrices.hpp"

#undef PREFIX
#undef POLICY
