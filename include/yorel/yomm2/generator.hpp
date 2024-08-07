// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED
#define YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED

#include <yorel/yomm2/core.hpp>

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
    void add_forward_declaration(const std::type_info& type);
    template<typename T>
    void add_forward_declaration();
    void add_forward_declaration(std::string_view name);
    template<class Policy>
    void add_forward_declarations();
    void write_forward_declarations(std::ostream& os) const;
    template<class Policy>
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

inline void generator::add_forward_declaration(std::string_view type) {
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
}

inline void generator::add_forward_declaration(const std::type_info& type) {
    add_forward_declaration(boost::core::demangle(type.name()));
}

template<typename T>
inline void generator::add_forward_declaration() {
    add_forward_declaration(typeid(T));
}

template<class Policy>
void generator::add_forward_declarations() {
    for (auto& method : Policy::methods) {
        add_forward_declaration(
            *reinterpret_cast<const std::type_info*>(method.method_type));
    }
}
inline void generator::write_forward_declarations(std::ostream& os) const {
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
}

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
    //    create_dispatch_data(compiler, slots, tables);

    os << std::hex << std::showbase;

    auto slots_and_strides_size = std::accumulate(
        compiler.methods.begin(), compiler.methods.end(), size_t(0),
        [](auto sum, auto& method) { return sum + 2 * method.arity() - 1; });
    auto dispatch_tables_size = std::accumulate(
        compiler.methods.begin(), compiler.methods.end(), size_t(0),
        [](auto sum, auto& method) {
            return sum + method.dispatch_table.size();
        });

    size_t encode_vtbl_size = 0, decode_vtbl_size = 0;

    for (auto& cls : compiler.classes) {
        size_t encode_size = 0;

        for (auto& entry :
             range(cls.vtbl.begin() + cls.first_used_slot, cls.vtbl.end())) {
            if (entry.vp_index > 1) {
                // It's a multi-method, and not the first virtual parameter.
                // Encode the index, it will be decoded as is.
                ++encode_size;
            } else {
                // It's a uni-method, or the first virtual parameter of a
                // multi-method. Encode the method index and the spec index;
                encode_size += 2;
            }
        }

        encode_vtbl_size += encode_size;
        decode_vtbl_size += cls.vtbl.size() - cls.first_used_slot;
    }

    char prelude_format[] = R"(
    static union {
        struct {
            uint16_t headroom[%d];
            uint16_t slots[%d];
            uint16_t dispatch[%d];
            uint16_t vtbl[%d];
        } packed;
        std::uintptr_t decode[%d];
    } yomm2_dispatch_data = { { {}, {
)";

    const auto total_decode_size = dispatch_tables_size + decode_vtbl_size;
    const auto total_encode_size =
        slots_and_strides_size + dispatch_tables_size + encode_vtbl_size;

    const auto headroom = total_decode_size * sizeof(uintptr_t) -
        total_encode_size * sizeof(uint16_t);
    char prelude[sizeof(prelude_format) + 5 * 6];
    std::snprintf(
        prelude, sizeof(prelude), prelude_format, headroom / sizeof(uint16_t),
        slots_and_strides_size, dispatch_tables_size, encode_vtbl_size,
        total_decode_size);
    os << prelude;

    std::vector<const generic_compiler::method*> methods;
    methods.resize(compiler.methods.size());
    std::transform(
        compiler.methods.begin(), compiler.methods.end(), methods.begin(),
        [](auto& method) { return &method; });

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

    os << "    }, {\n";

    std::vector<std::vector<uint16_t>> vtbls(compiler.classes.size());

    for (auto& cls : range(compiler.classes.begin(), compiler.classes.end())) {
        os << indent << "// "
           << boost::core::demangle(
                  reinterpret_cast<const std::type_info*>(cls.type_ids[0])
                      ->name())
           << "\n";

        os << indent << cls.first_used_slot << ", ";

        for (auto& entry : cls.vtbl) {
            os << "    ";
            auto stop = &entry == &cls.vtbl.back() ? stop_bit : 0;

            if (entry.vp_index > 0) {
                // It's a multi-method, and not the first virtual parameter.
                // Encode the index, it will be decoded as is.
                os << (entry.group_index | index_bit | stop);
            } else {
                // It's a uni-method, or the first virtual parameter of a
                // multi-method. The group is an index into a linear table of
                // specs.
                auto method = methods[entry.method_index];
                auto spec = method->dispatch_table[entry.group_index];
                // Encode the method index and the spec index;
                os << entry.method_index << ", " << (spec->spec_index | stop);
            }

            os << ", ";
        }

        os << "\n";
    }

    os << "\n    } } };\n\n";
    os << "    yorel::yomm2::detail::decode_dispatch_data<"
       << (policy.empty() ? "YOMM2_DEFAULT_POLICY" : policy)
       << ">(yomm2_dispatch_data);\n\n";
}

} // namespace yomm2
} // namespace yorel

#endif
