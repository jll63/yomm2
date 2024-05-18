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

    *out++ = std::string_view(first.operator(), iter - first);

    return out;
}

} // namespace detail

template<class Policy>
class generator {
  public:
    explicit generator(
        const compiler<Policy>& comp, std::filesystem::path file);
    explicit generator(const compiler<Policy>& comp, std::ostream& os);

    void write_forward_declarations() const;
    void write_static_offsets() const;

  private:
    const compiler<Policy>& comp;
    std::filesystem::path file;
    std::ofstream ofs;
    std::ostream& os;
};

template<class Policy>
generator(const compiler<Policy>& comp, std::filesystem::path file)
    -> generator<Policy>;

template<class Policy>
generator(const compiler<Policy>& comp, std::ostream& os) -> generator<Policy>;

template<class Policy>
generator<Policy>::generator(
    const compiler<Policy>& comp, std::filesystem::path file)
    : comp(comp), file(file), os(ofs) {
    std::string temp_file(file.string() + ".tmp");
    ofs.open(temp_file);
}

template<class Policy>
generator<Policy>::generator(const compiler<Policy>& comp, std::ostream& os)
    : comp(comp), os(os) {
}

template<class Policy>
void generator<Policy>::write_forward_declarations() const {
    std::vector<std::string> names;

    auto out = std::transform(
        comp.classes.begin(), comp.classes.end(),
        std::back_insert_iterator(names), [](auto& cls) {
            return boost::core::demangle(
                reinterpret_cast<const std::type_info*>(cls.type_ids[0])
                    ->name());
        });

    out = std::transform(
        comp.methods.begin(), comp.methods.end(), out, [](auto& method) {
            auto name =
                boost::core::demangle(reinterpret_cast<const std::type_info*>(
                                          method.info->method_type)
                                          ->name());

            // By construction, 'name' is in the form 'method<ID, ...>'. ID can be a
            // simple name, or not, if it is a template class - like
            // 'test_policy_<int Key>'. But ID is well-formed, otherwise the program
            // would not have compiled. Let's extract ID.

            auto key_first = name.begin() + name.find("<") + 1;
            auto key_last = std::find(key_first, name.end(), ',');

            return std::string(key_first, key_last);
        });

    for (auto& method : comp.methods) {
        auto method_name = boost::core::demangle(
            reinterpret_cast<const std::type_info*>(method.info->method_type)
                ->name());

        auto iter = method_name.begin(), name_begin = method_name.end();

        while (iter != method_name.end()) {
            if (!(std::isalnum(*iter) || *iter == '_' || *iter == ':')) {
            }
        }
    }

    if (names.empty()) {
        return;
    }

    std::sort(names.begin(), names.end());
    std::vector<std::vector<std::string_view>> paths;
    std::string_view scope("::");

    for (auto& name : names) {
        std::vector<std::string_view> components;
        auto pos = 0;
        auto next = name.find(scope);

        while (true) {
            if (next == std::string::npos) {
                components.emplace_back(name.data() + pos, name.length() - pos);
                break;
            } else {
                components.emplace_back(name.data() + pos, next - pos);
                pos = next + scope.length();
                next = name.find(scope, pos);
            }
        }

        paths.push_back(std::move(components));
    }

    using detail::range;
    size_t nesting = 0;

    for (auto& ns : range(paths.begin()->begin(), paths.begin()->end() - 1)) {
        os << "namespace " << ns << " {\n";
    }

    os << "struct " << paths.front().back() << ";\n";

    auto prev_iter = paths.begin();

    for (auto& cur : range(paths.begin() + 1, paths.end())) {
        auto& prev = *prev_iter;

        if (prev == cur) {
            continue;
        }

        // Work with indices, not iterators. It makes debugging easier, and the
        // compiler will optimize anyway.
        size_t i = 0, n = prev.size() - 1;
        size_t j = 0, m = cur.size() - 1;

        while (i != n && j != m && prev[i] == cur[j]) {
            ++i;
            ++j;
        }

        while (i != n) {
            os << "}\n";
            ++i;
        }

        while (j < m) {
            os << "namespace " << cur[j] << " {\n";
            ++j;
        }

        os << "struct " << cur.back() << ";\n";

        ++prev_iter;
    }

    for (auto close = paths.back().size() - 1; close; --close) {
        os << "}\n";
    }
}

template<class Policy>
void generator<Policy>::write_static_offsets() const {
    for (auto& method : comp.methods) {
        auto method_name = boost::core::demangle(
            reinterpret_cast<const std::type_info*>(method.info->method_type)
                ->name());
        os << "template<> struct ::yorel::yomm2::detail::static_offsets<"
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

    os << "} } }\n";
}

} // namespace yomm2
} // namespace yorel

#endif
