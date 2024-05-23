// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED
#define YOREL_YOMM2_GENERATOR_GENERATE_INCLUDED

#include <yorel/yomm2/compiler.hpp>

#include <iostream>
#include <iterator>
#include <fstream>
#include <filesystem>
#include <set>

namespace yorel {
namespace yomm2 {

namespace detail {

template<typename Iter>
Iter extract_type(Iter iter) {
    auto depth = 0;

    do {
        switch (*iter) {
        case '<':
        case '[':
        case '(':
            ++depth;
            break;

        case '>':
        case ']':
        case ')':
            if (--depth == 0) {
                return iter;
            }
        }

        ++iter;
    } while (depth > 0);

    return iter; // never reached - make compiler happy
}

template<typename Input, typename Output>
Output extract_simple_types(Input first, Input last, Output out) {
    auto iter = first;

    while (std::isalnum(*iter) || *iter == '_' || *iter == ':') {
        ++iter;
    }

    *out++ = std::string_view(&*first, iter - first);

    return out;
}

} // namespace detail

class generator {
  public:
    explicit generator(std::filesystem::path file);
    explicit generator(std::ostream& os);

    void add(std::string_view name);
    void add(const std::type_info& type);
    template<class... T>
    void add();

    void write_forward_declarations() const;
    void write_static_offsets() const;

  private:
    std::filesystem::path file;
    std::ofstream ofs;
    std::ostream& os;
    std::set<std::string> names;
};

inline generator::generator(std::filesystem::path file) : file(file), os(ofs) {
    std::string temp_file(file.string() + ".tmp");
    ofs.open(temp_file);
}

inline generator::generator(std::ostream& os) : os(os) {
}

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

inline std::unordered_set<std::string_view> built_in_types = {
    "int", "unsigned", "void"};

} // namespace detail

inline void generator::add(std::string_view type) {
    using namespace detail;

    auto iter = type.begin(), last = type.end();

    while (true) {
        auto name_first = std::find_if(
            iter, last, [](char c) { return std::isalnum(c) || c == '_'; });

        if (name_first == last) {
            break;
        }

        auto name_last = std::find_if(name_first, last, [](char c) {
            return !(std::isalnum(c) || c == '_' || c == ':');
        });

        iter = name_last;

        if (iter != last && *iter == '<') {
            ++iter;
            continue;
        }

        std::string_view name(&*name_first, name_last - name_first);

        if (starts_with(name, "std::") || starts_with(name, "yorel::")) {
            continue;
        }

        if (built_in_types.find(name) != built_in_types.end()) {
            continue;
        }

        names.emplace(name);
    }
}

inline void generator::add(const std::type_info& type) {
    add(boost::core::demangle(type.name()));
}

template<class... T>
void generator::add() {
    (add(boost::core::demangle(typeid(T).name())), ...);
}

inline void generator::write_forward_declarations() const {
    const std::string file_scope;
    auto prev_ns_iter = file_scope.begin();
    auto prev_ns_last = file_scope.begin();

    for (auto& name : names) {
        //           v=prev_ns_last
        // foo::bar::x
        // fx

        //           v=prev_ns_last
        // foo::bar::x
        // foo::bx

        // v=prev_ns_last
        // x
        // y

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

inline void generator::write_static_offsets() const {
    // for (auto& method : comp.methods) {
    //     auto method_name = boost::core::demangle(
    //         reinterpret_cast<const std::type_info*>(method.info->method_type)
    //             ->name());
    //     os << "template<> struct ::yorel::yomm2::detail::static_offsets<"
    //        << method_name << "> {static constexpr size_t slots[] = {";

    //     const auto arity = method.info->arity();
    //     auto comma = "";

    //     for (auto slot : method.slots) {
    //         os << comma << slot;
    //         comma = ", ";
    //     }

    //     if (arity > 1) {
    //         os << "}; static constexpr size_t strides[] = {";
    //         comma = "";

    //         for (auto stride : method.strides) {
    //             os << comma << stride;
    //             comma = ", ";
    //         }
    //     }

    //     os << "}; };\n";
    // }

    // os << "} } }\n";
}

} // namespace yomm2
} // namespace yorel

#endif
