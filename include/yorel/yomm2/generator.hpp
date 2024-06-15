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
    void open(std::filesystem::path path);
    void open(std::ostream& os);
    void close();

    void add(const generic_compiler& compiler);
    void add(std::string_view name);
    void add(const std::type_info& type);
    template<class... T>
    void add();

    void forward_declarations() const;
    void static_offsets(const generic_compiler::method& method) const;
    void static_offsets(const generic_compiler& compiler) const;

    static constexpr auto temp_ext = ".yomm2.tmp";

  private:
    static std::unordered_set<std::string_view> tokens;
    std::filesystem::path path;
    std::ofstream ofs;
    std::ostream* os;
    std::set<std::string> names;
};

inline void generator::open(std::filesystem::path path) {
    this->path = path;
    os = &ofs;

    std::string temp(path.c_str());
    temp += temp_ext;
    ofs.open(temp);
}

inline void generator::open(std::ostream& os) {
    close();
    this->os = &os;
}

inline void generator::close() {
    if (os == &ofs) {
        namespace fs = std::filesystem;

        std::string temp = path;
        temp += temp_ext;
        std::ifstream olds(path);
        std::ifstream curs(temp);

        if (olds && curs) {
            ofs.close();
            bool same = true;
            std::string old, cur;

            do {
                if (!std::getline(olds, old)) {
                    same = !std::getline(curs, cur);
                    break;
                }

                if (!std::getline(curs, cur)) {
                    same = false;
                    break;
                }

                same = cur == old;
            } while (same);

            if (same) {
                fs::remove(temp);
            } else {
                fs::rename(temp, path);
            }
        } else {
            fs::rename(temp, path);
            ofs.close();
        }
    }

    this->path.clear();
    os = nullptr;
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

template<typename Iter>
Iter name_end(Iter first, Iter last) {
    return std::find_if(first, last, [](char c) {
        return !(std::isalnum(c) || c == '_' || c == ':');
    });
}
} // namespace detail

inline std::unordered_set<std::string_view> generator::tokens = {
    "void",   "bool",  "char", "int",    "float",
    "double", "short", "long", "signed", "unsigned",
};

inline void generator::add(const generic_compiler& compiler) {
    for (auto& method : compiler.methods) {
        add(*reinterpret_cast<const std::type_info*>(method.info->method_type));
    }
}

inline void generator::add(std::string_view type) {
    using namespace detail;

    std::regex name_regex(R"((\w+(?:::\w+)*)( *<)?)");

    auto iter = std::regex_iterator(type.begin(), type.end(), name_regex);
    auto words_end = std::sregex_iterator();

    for (decltype(iter) last; iter != last; ++iter) {
        if ((*iter)[2].matched) {
            continue;
        }

        auto match = (*iter)[1];
        std::string_view name(match.first, match.length());

        if (!match.matched) {
            continue;
        }

        if (!std::isalpha(*match.first)) {
            continue;
        }

        if (tokens.find(name) != tokens.end()) {
            continue;
        }

        if (starts_with(name, "std::") || starts_with(name, "yorel::")) {
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

inline void generator::forward_declarations() const {
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
                        *os << "}\n";
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
                *os << "class "
                    << std::string_view(&*name_iter, scope_iter - name_iter)
                    << ";\n";
                break;
            } else {
                *os << "namespace "
                    << std::string_view(&*name_iter, scope_iter - name_iter)
                    << " {\n";
                name_iter = scope_iter + 2;
                prev_ns_last = name_iter;
            }
        }
    }

    while (prev_ns_iter != prev_ns_last) {
        if (*prev_ns_iter == ':') {
            *os << "}\n";
            ++prev_ns_iter;
        }

        ++prev_ns_iter;
    }
}

void generator::static_offsets(const generic_compiler& compiler) const {
    for (auto& method : compiler.methods) {
        static_offsets(method);
    }
}

void generator::static_offsets(const generic_compiler::method& method) const {
    auto method_name = boost::core::demangle(
        reinterpret_cast<const std::type_info*>(method.info->method_type)
            ->name());
    *os << "template<> struct yorel::yomm2::detail::static_offsets<"
        << method_name << "> {static constexpr size_t slots[] = {";

    const auto arity = method.info->arity();
    auto comma = "";

    for (auto slot : method.slots) {
        *os << comma << slot;
        comma = ", ";
    }

    if (arity > 1) {
        *os << "}; static constexpr size_t strides[] = {";
        comma = "";

        for (auto stride : method.strides) {
            *os << comma << stride;
            comma = ", ";
        }
    }

    *os << "}; };\n";
}

} // namespace yomm2
} // namespace yorel

#endif
