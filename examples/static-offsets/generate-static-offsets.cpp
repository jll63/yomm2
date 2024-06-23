#include "animals.hpp"

#include <fstream>

#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/generator.hpp>

int main(int argc, char* argv[]) {
    yorel::yomm2::compiler compiler;
    compiler.compile();

    std::ofstream os(argv[1]);
    yorel::yomm2::generator generator;
    generator.add_forward_declarations(compiler);
    generator.write_forward_declarations(os);
    generator.write_static_offsets(compiler, os);

    return 0;
}
