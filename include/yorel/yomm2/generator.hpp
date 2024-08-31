// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED
#define YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>

#include <boost/core/demangle.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <regex>
#include <set>

namespace yorel {
namespace yomm2 {

class generator {
  public:
#ifndef _MSC_VER
    generator& add_forward_declaration(const std::type_info& type);
    template<typename T>
    generator& add_forward_declaration();
    generator& add_forward_declaration(std::string_view name);
    template<class Policy = YOMM2_DEFAULT_POLICY>
    generator& add_forward_declarations();
    const generator& write_forward_declarations(std::ostream& os) const;
#endif
    template<class Policy = YOMM2_DEFAULT_POLICY>
    const generator& write_static_offsets(std::ostream& os) const;
    template<class Compiler>
    static void
    encode_dispatch_data(const Compiler& compiler, std::ostream& os);
    template<class Compiler>
    static void encode_dispatch_data(
        const Compiler& compiler, const std::string& policy, std::ostream& os);

  private:
    void write_static_offsets(
        const detail::method_info& method, std::ostream& os) const;

    static uint16_t encode_group(
        const detail::generic_compiler::method* method,
        const detail::generic_compiler::vtbl_entry& entry);

    static std::unordered_set<std::string_view> keywords;
    std::set<std::string> names;
};

namespace detail {

inline bool starts_with(std::string_view name, const char* prefix) {
    // Assume that prefix is not an empty string.

    for (auto c : name) {
        if (c != *prefix) {
            return false;
        }

        ++prefix;

        if (!*prefix) {
            return true;
        }
    }

    return false;
}

} // namespace detail

// clang-format off
inline std::unordered_set<std::string_view> generator::keywords = {
    "void",   "bool",  "char", "int",    "float",
    "double", "short", "long", "signed", "unsigned",
    "class", "struct", "enum",
};
// clang-format on

#ifndef _MSC_VER

inline generator& generator::add_forward_declaration(std::string_view type) {
    using namespace detail;

    std::regex name_regex(R"((\w+(?:::\w+)*)( *<)?)");

    auto iter = std::regex_iterator(type.begin(), type.end(), name_regex);
    auto words_end = std::sregex_iterator();

    for (decltype(iter) last; iter != last; ++iter) {
        if ((*iter)[2].matched) {
            continue;
        }

        auto match = (*iter)[1];

        if (!match.matched) {
            continue;
        }

        std::string_view name(&*match.first, match.length());

        if (!std::isalpha(*match.first)) {
            continue;
        }

        if (keywords.find(name) != keywords.end()) {
            continue;
        }

        if (starts_with(name, "std::") || starts_with(name, "yorel::")) {
            continue;
        }

        names.emplace(name);
    }

    return *this;
}

inline generator&
generator::add_forward_declaration(const std::type_info& type) {
    return add_forward_declaration(boost::core::demangle(type.name()));
}

template<typename T>
inline generator& generator::add_forward_declaration() {
    return add_forward_declaration(typeid(T));
}

template<class Policy>
generator& generator::add_forward_declarations() {
    for (auto& method : Policy::methods) {
        add_forward_declaration(
            *reinterpret_cast<const std::type_info*>(method.method_type));
    }

    return *this;
}

inline const generator&
generator::write_forward_declarations(std::ostream& os) const {
    const std::string file_scope;
    auto prev_ns_iter = file_scope.begin();
    auto prev_ns_last = file_scope.begin();

    for (auto& name : names) {
        auto name_iter = name.begin();
        auto ns_last = name_iter;

        while (prev_ns_iter != prev_ns_last) {
            if (name_iter == name.end() || *prev_ns_iter != *name_iter) {
                while (prev_ns_iter != prev_ns_last) {
                    if (*prev_ns_iter == ':') {
                        os << "}\n";
                        ++prev_ns_iter;
                    }
                    ++prev_ns_iter;
                }

                while (name_iter != name.begin() && name_iter[-1] != ':') {
                    --name_iter;
                }

                ns_last = name_iter;

                break;
            }

            ++prev_ns_iter;
            ++name_iter;
        }

        prev_ns_iter = name.begin();
        prev_ns_last = name_iter;

        while (true) {
            auto scope_iter = std::find(name_iter, name.end(), ':');

            if (scope_iter == name.end()) {
                os << "class "
                   << std::string_view(&*name_iter, scope_iter - name_iter)
                   << ";\n";
                break;
            } else {
                os << "namespace "
                   << std::string_view(&*name_iter, scope_iter - name_iter)
                   << " {\n";
                name_iter = scope_iter + 2;
                prev_ns_last = name_iter;
            }
        }
    }

    while (prev_ns_iter != prev_ns_last) {
        if (*prev_ns_iter == ':') {
            os << "}\n";
            ++prev_ns_iter;
        }

        ++prev_ns_iter;
    }

    return *this;
}

#endif

template<class T>
const generator& generator::write_static_offsets(std::ostream& os) const {
    using namespace detail;

    if constexpr (is_policy<T>) {
        for (auto& method : T::methods) {
            write_static_offsets(method, os);
        }
    } else {
        static_assert(is_method<T>);
        write_static_offsets(T::fn, os);
    }

    return *this;
}

void generator::write_static_offsets(
    const detail::method_info& method, std::ostream& os) const {
    using namespace detail;

    auto method_name = boost::core::demangle(
        reinterpret_cast<const std::type_info*>(method.method_type)->name());
    os << "template<> struct yorel::yomm2::detail::static_offsets<"
       << method_name << "> {static constexpr size_t slots[] = {";

    os << method.slots_strides_ptr[0];

    if (method.arity() > 1) {
        for (size_t i = 1; i < method.arity(); i++) {
            os << ", " << method.slots_strides_ptr[i * 2 - 1];
        }

        os << "}; static constexpr size_t strides[] = {";
        auto comma = "";

        for (size_t i = 1; i < method.arity(); i++) {
            os << comma << method.slots_strides_ptr[i * 2];
            comma = ", ";
        }
    }

    os << "}; };\n";
}

uint16_t generator::encode_group(
    const detail::generic_compiler::method* method,
    const detail::generic_compiler::vtbl_entry& entry) {
    auto spec = method->dispatch_table[entry.group_index];

    if (method->arity() == 1) {
        auto spec = method->dispatch_table[entry.group_index];

        if (spec == &method->not_implemented) {
            return method->specs.size();
        } else if (spec == &method->ambiguous) {
            return method->specs.size() + 1;
        } else {
            return entry.group_index;
        }
    } else {
        return entry.group_index;
    }
}

template<class Compiler>
void generator::encode_dispatch_data(
    const Compiler& compiler, std::ostream& os) {
    encode_dispatch_data(compiler, "YOMM2_DEFAULT_POLICY", os);
}

template<class Compiler>
void generator::encode_dispatch_data(
    const Compiler& compiler, const std::string& policy, std::ostream& os) {
    const char* indent = "        ";
    using namespace yorel::yomm2::detail;

    std::vector<std::size_t> slots;
    std::vector<std::uintptr_t> tables;

    // -------------------------------------------------------------------------
    // Calculate data sizes.

    auto slots_and_strides_size = std::accumulate(
        compiler.methods.begin(), compiler.methods.end(), size_t(0),
        [](auto sum, auto& method) { return sum + 2 * method.arity() - 1; });
    auto dispatch_tables_size = std::accumulate(
        compiler.methods.begin(), compiler.methods.end(), size_t(0),
        [](auto sum, auto& method) {
            if (method.arity() == 1) {
                return sum;
            } else {
                return sum + method.dispatch_table.size();
            }
        });

    size_t encode_vtbl_size = 0, decode_vtbl_size = 0;

    for (auto& cls : compiler.classes) {
        ++encode_vtbl_size; // for first slot index

        for (auto& entry : cls.vtbl) {
            if (entry.vp_index != 0) {
                // It's a multi-method, and not the first virtual parameter.
                // Encode the index, it will be decoded as is.
                ++encode_vtbl_size;
            } else {
                // It's a uni-method, or the first virtual parameter of a
                // multi-method. Encode the method index and the spec index;
                encode_vtbl_size += 2;
            }
        }

        decode_vtbl_size += cls.vtbl.size() - cls.first_slot;
    }

    // -------------------------------------------------------------------------
    // Write data structure declaration and open initializer.

    char prelude_format[] = R"(
    static struct {
        union {
            struct {
                uint16_t headroom[%d];
                uint16_t slots[%d];
                uint16_t vtbls[%d];
            } encoded;
            std::uintptr_t vtbls[%d];
        };
        std::uintptr_t dtbls[%d];
    } yomm2_dispatch_data = { { { {}, {
)";

    const auto decode_size = sizeof(uintptr_t);
    const auto encode_size = sizeof(uint16_t);

    const auto total_decode_size = decode_vtbl_size;
    const auto total_encode_size = slots_and_strides_size + encode_vtbl_size;

    const auto headroom =
        (decode_vtbl_size * decode_size -
         (encode_vtbl_size - slots_and_strides_size) * encode_size) /
        encode_size;

    char prelude[sizeof(prelude_format) + 5 * 6];
    std::snprintf(
        prelude, sizeof(prelude), prelude_format, headroom,
        slots_and_strides_size, encode_vtbl_size, decode_vtbl_size,
        dispatch_tables_size);
    os << prelude;

    std::vector<const generic_compiler::method*> methods;
    methods.resize(compiler.methods.size());
    std::transform(
        compiler.methods.begin(), compiler.methods.end(), methods.begin(),
        [](auto& method) { return &method; });

    // -------------------------------------------------------------------------
    // Write slots and strides.

    // Write hex from now on.
    os << std::hex << std::showbase;
    os << indent << "// slots and strides\n";

    for (auto& method : methods) {
        os << indent << "// "
           << boost::core::demangle(method->info->name.data()) << "\n";
        os << indent;
        auto strides_iter = std::transform(
            method->slots.begin(), method->slots.end(),
            std::ostream_iterator<uint16_t>(os, ", "),
            [](auto slot) { return uint16_t(slot); });
        std::transform(
            method->strides.begin(), method->strides.end(),
            std::ostream_iterator<uint16_t>(os, ", "),
            [](auto slot) { return uint16_t(slot); });
        os << "\n";
    }

    os << "    }, {\n";

    // -------------------------------------------------------------------------
    // Write v-tables.

    os << indent << "// v-tables\n";

    for (auto& cls : compiler.classes) {
        os << indent << "// "
           << boost::core::demangle(
                  reinterpret_cast<const std::type_info*>(cls.type_ids[0])
                      ->name())
           << "\n";

        os << indent << cls.first_slot << ", // first used slot\n";

        for (auto& entry : cls.vtbl) {
            os << indent;
            auto stop = &entry == &cls.vtbl.back() ? stop_bit : 0;

            if (entry.vp_index > 0) {
                // It's a multi-method, and not the first virtual parameter.
                // Encode the index, it will be decoded as is.
                os << uint16_t(entry.group_index | index_bit | stop);
            } else {
                // It's a uni-method, or the first virtual parameter of a
                // multi-method. The group is an index into a linear table of
                // specs.
                auto method = methods[entry.method_index];
                os << uint16_t(entry.method_index) << ", ";

                if (method->info->arity() == 1) {
                    // For 1-methods, there is no dispatch table, the v-table
                    // slot contains a pointer to function. Encode the spec
                    // index.
                    auto spec = method->dispatch_table[entry.group_index];
                    os << uint16_t(spec->spec_index | stop);
                } else {
                    // For multi-methods, the v-table slot contains a pointer to
                    // a row in the dispatch table. Enocde the row index.
                    os << uint16_t(entry.group_index | stop);
                }
            }

            os << ",\n";
        }
    }

    os << "   } } }, {\n";

    // -------------------------------------------------------------------------
    // Write multi-methods dispatch tables.

    os << indent << "// multi-methods dispatch tables\n";

    for (auto& method :
         range(compiler.methods.begin(), compiler.methods.end())) {
        if (method.arity() < 2) {
            continue;
        }

        os << indent << "// " << boost::core::demangle(method.info->name.data())
           << "\n";
        os << indent;
        auto dt_iter = std::transform(
            method.dispatch_table.begin(), method.dispatch_table.end() - 1,
            std::ostream_iterator<uint16_t>(os, ", "),
            [&method](auto entry) { return uint16_t(entry->spec_index); });
        auto& last = method.dispatch_table.back();
        *dt_iter = (uint16_t)last->spec_index | stop_bit;
        os << "\n";
    }

    // -------------------------------------------------------------------------
    // Close braces.

    os << "    } };\n\n";

    // -------------------------------------------------------------------------
    // Write call to decoder.
    os << "    yorel::yomm2::detail::decode_dispatch_data<"
       << (policy.empty() ? "YOMM2_DEFAULT_POLICY" : policy)
       << ">(yomm2_dispatch_data);\n\n";
}

} // namespace yomm2
} // namespace yorel

#endif
