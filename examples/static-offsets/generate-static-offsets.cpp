#include "animals.hpp"

#include <iostream>

#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/generator.hpp>

int main(int argc, char* argv[]) {
    yorel::yomm2::compiler compiler;
    compiler.compile();

    yorel::yomm2::generator generator;
    generator.add_forward_declarations(compiler);
    generator.write_forward_declarations(std::cout);
    generator.write_static_offsets(compiler, std::cout);

    return 0;
}
