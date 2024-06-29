// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED
#define YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>

#include <cassert>
#include <iostream>
#include <iterator>
#include <fstream>
#include <filesystem>
#include <regex>
#include <set>

namespace yorel {
namespace yomm2 {

class generator {
  public:
    void add_forward_declaration(const std::type_info& type);
    template<class... T>
    void add_forward_declarations();
    void add_forward_declarations(const generic_compiler& compiler);
    void add_forward_declaration(std::string_view name);
    void write_forward_declarations(std::ostream& os) const;
    void write_static_offsets(
        const generic_compiler& compiler, std::ostream& os) const;

  private:
    void write_static_offsets(
        const generic_compiler::method& method, std::ostream& os) const;

    static std::unordered_set<std::string_view> keywords;
    std::set<std::string> names;
};

namespace detail {

inline bool starts_with(std::string_view name, const char* prefix) {
    // Assumes that prefix is not an empty string.

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

inline std::unordered_set<std::string_view> generator::keywords = {
    "void",   "bool",  "char", "int",    "float",
    "double", "short", "long", "signed", "unsigned",
    "class", "struct", "enum",
};

inline void
generator::add_forward_declarations(const generic_compiler& compiler) {
    for (auto& method : compiler.methods) {
        add_forward_declaration(
            *reinterpret_cast<const std::type_info*>(method.info->method_type));
    }
}

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

template<class... T>
void generator::add_forward_declarations() {
    (add_forward_declaration(boost::core::demangle(typeid(T).name())), ...);
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

void generator::write_static_offsets(
    const generic_compiler& compiler, std::ostream& os) const {
    for (auto& method : compiler.methods) {
        write_static_offsets(method, os);
    }
}

void generator::write_static_offsets(
    const generic_compiler::method& method, std::ostream& os) const {
    auto method_name = boost::core::demangle(
        reinterpret_cast<const std::type_info*>(method.info->method_type)
            ->name());
    os << "template<> struct yorel::yomm2::detail::static_offsets<"
       << method_name << "> {static constexpr size_t slots[] = {";

    const auto arity = method.info->arity();
    auto comma = "";

    for (auto slot : method.slots) {
        os << comma << slot;
        comma = ", ";
    }

    if (arity > 1) {
        os << "}; static constexpr size_t strides[] = {";
        comma = "";

        for (auto stride : method.strides) {
            os << comma << stride;
            comma = ", ";
        }
    }

    os << "}; };\n";
}

} // namespace yomm2
} // namespace yorel

#endif
