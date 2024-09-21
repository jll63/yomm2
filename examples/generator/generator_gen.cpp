#include "animals.hpp"

#include <iostream>

#include <yorel/yomm2/generator.hpp>

int main(int argc, char* argv[]) {
    using namespace yorel::yomm2;

    auto compiler = initialize();
    generator generator;

    std::ofstream slots("slots.hpp");
    generator
        .write_static_offsets<method_class(kick, (virtual_ptr<Animal>), void)>(
            slots)
        .write_static_offsets<method_class(
            meet, (virtual_ptr<Animal>, virtual_ptr<Animal>), void)>(slots);

    std::ofstream tables("tables.hpp");
    generator.encode_dispatch_data(compiler, tables);

    return 0;
}
