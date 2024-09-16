#include "animals.hpp"

#include <iostream>

#include <yorel/yomm2/generator.hpp>

int main(int argc, char* argv[]) {
    using namespace yorel::yomm2;

    auto compiler = initialize();
    generator generator;

    std::ofstream slots("slots.hpp");
    generator
        .write_static_offsets<method_class(void, kick, (virtual_ptr<Animal>))>(
            slots)
        .write_static_offsets<method_class(
            void, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>))>(slots);

    std::ofstream tables("tables.hpp");
    generator.encode_dispatch_data(compiler, tables);

    return 0;
}
