// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_COMPILER_GENERATE_INCLUDED
#define YOREL_YOMM2_COMPILER_GENERATE_INCLUDED

#include <fstream>
#include <filesystem>

#include <yorel/yomm2/compiler.hpp>

namespace yorel {
namespace yomm2 {

template<class Policy>
template<typename Stream>
void compiler<Policy>::generate_forward_declarations(Stream& os) const {
    std::vector<std::string> names(
        std::distance(classes.begin(), classes.end()) +
        std::distance(methods.begin(), methods.end()));

    auto out = std::transform(
        classes.begin(), classes.end(), names.begin(), [](auto& cls) {
            return boost::core::demangle(
                reinterpret_cast<const std::type_info*>(cls.type_ids[0])
                    ->name());
        });

    out = std::transform(methods.begin(), methods.end(), out, [](auto& method) {
        auto name = boost::core::demangle(
            reinterpret_cast<const std::type_info*>(method.info->method_type)
                ->name());

        // By construction, 'name' is in the form 'method<ID, ...>'. ID can be a
        // simple name, or not, if it is a template class - like
        // 'test_policy_<int Key>'. But ID is well-formed, otherwise the program
        // would not have compiled. Let's extract ID.

        auto key_first = name.begin() + name.find("<") + 1;
        auto key_last = std::find(key_first, name.end(), ',');

        return std::string(key_first, key_last);
    });

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
template<typename Stream>
void compiler<Policy>::generate_static_offsets(Stream& os) const {
    for (auto& method : methods) {
        auto method_name = boost::core::demangle(
            reinterpret_cast<const std::type_info*>(method.info->method_type)
                ->name());
        os << "template<> struct static_offsets<" << method_name
           << "> {static constexpr size_t slots[] = {";

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

template<class Policy>
template<typename Stream>
void compiler<Policy>::generate_header(Stream& os) const {
    generate_forward_declarations(os);
    generate_static_offsets(os);
}

template<class Policy>
void compiler<Policy>::generate_header(std::string_view str_path) const {
    std::filesystem::path path(str_path);
    std::string temp_path(path.string() + ".tmp");
    std::ofstream temp(temp_path);
    generate_header(temp);
}

} // namespace yomm2
} // namespace yorel

#endif
