// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "test_generator_domain.hpp"

#include <iostream>
#include <yorel/yomm2/generator.hpp>

int main(int argc, char* argv[]) {
    using namespace yorel::yomm2;

    auto compiler = update<throw_policy>();
    generator generator;

    std::ofstream slots("test_generator_slots.hpp");
#ifndef _MSC_VER
    generator.add_forward_declarations().write_forward_declarations(slots);
#endif
    generator.write_static_offsets(slots);

    std::ofstream tables("test_generator_tables.hpp");
    generator.encode_dispatch_data(compiler, tables);

    return 0;
}
