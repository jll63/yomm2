#ifndef YOREL_YOMM2_DECODE_HPP
#define YOREL_YOMM2_DECODE_HPP

#include <yorel/yomm2/detail/trace.hpp>

namespace yorel {
namespace yomm2 {

constexpr std::uint16_t stop_bit = 1 << (sizeof(uint16_t) * 8 - 1);
constexpr std::uint16_t index_bit = stop_bit >> 1;

template<class Policy, typename Data>
void decode_dispatch_data(Data& init) {
    using namespace yorel::yomm2::detail;

    constexpr auto pointer_size = sizeof(std::uintptr_t);

    trace_type<Policy> trace;
    using indent = typename trace_type<Policy>::indent;

    trace << "Decoding dispatch data for "
          << type_name(Policy::template static_type<Policy>()) << "\n";

    auto method_count = 0, multi_method_count = 0;

    for (auto& method : Policy::methods) {
        ++method_count;

        if (method.arity() >= 2) {
            ++multi_method_count;
        }
    }

    ++trace << method_count << " methods, " << multi_method_count
            << " multi-methods\n";

    // First copy the slots and strides to the static arrays in methods. Also
    // build an array of arrays of pointer to method definitions. Methods and
    // definitions are in reverse order, because of how 'list' works. While
    // building the array of array of defintions, we put them back in the order
    // in which the compiler saw them.
    auto packed_slots_iter = init.encoded.slots;
    auto methods = (method_info**)alloca(method_count * pointer_size);
    auto methods_iter = methods;
    auto method_defs = (uintptr_t**)alloca(method_count * pointer_size);
    auto method_defs_iter = method_defs;
    auto dispatch_tables =
        (std::uintptr_t**)alloca(method_count * pointer_size);
    auto multi_method_to_method =
        (std::size_t*)alloca(multi_method_count * sizeof(std::size_t));
    auto multi_method_to_method_iter = multi_method_to_method;

    {
        auto method_index = 0;

        for (auto& method : Policy::methods) {
            ++trace << "method " << type_name(method.method_type) << "\n";
            indent _(trace);

            *methods_iter++ = &method;

            ++trace << "specializations:\n";

            for (auto& spec : method.specs) {
                indent _(trace);
                ++trace << spec.pf << " " << type_name(spec.type) << "\n";
            }

            auto slots_strides_count = 2 * method.arity() - 1;

            // copy slots and strides into the method's static
            ++trace << "installing " << slots_strides_count
                    << " slots and strides\n";
            std::copy_n(
                packed_slots_iter, slots_strides_count,
                method.slots_strides_ptr);
            packed_slots_iter += slots_strides_count;

            auto specs =
                (uintptr_t*)alloca((method.specs.size() + 1) * pointer_size);
            *method_defs_iter++ = specs;
            ++trace << "specs index: " << specs << "\n";
            specs = std::transform(
                method.specs.begin(), method.specs.end(), specs,
                [](auto& spec) { return (uintptr_t)spec.pf; });
            *specs++ = (uintptr_t)method.not_implemented;
            ++method_index;
        }
    }

    // Decode dispatch tables for multi-methods, in place, and keep track of
    // them in an array. We will use it when we fill the vtables.

    ++trace << "decoding multi-method dispatch tables\n";
    {
        std::size_t method_index = 0;
        auto dtbl_iter = init.dtbls;

        for (auto& method : Policy::methods) {
            // Resist the temptation to use 'continue' to skip uni-methods, as
            // 'method_index' needs to be incremented.
            if (method.arity() > 1) {
                indent _(trace);

                dispatch_tables[method_index] = dtbl_iter;
                ++trace << "multi-method " << method_index
                        << " dispatch table at " << dtbl_iter << "\n";

                indent __(trace);
                ++trace << "specs:";

                auto defs = method_defs[method_index];
                bool more = true;

                while (more) {
                    more = !(*dtbl_iter & stop_bit);
                    auto spec_index = *dtbl_iter & ~stop_bit;
                    trace << " " << spec_index;
                    *dtbl_iter++ = defs[spec_index];
                };

                trace << "\n";
            }

            ++method_index;
        }
    }

    ++trace << "decoding v-tables\n";

    auto encode_iter = init.encoded.vtbls;
    auto decode_iter = init.vtbls;
    bool last;

    auto fetch = [&]() {
        BOOST_ASSERT((char*)(encode_iter + 1) >= (char*)decode_iter);
        auto code = *encode_iter++;
        last = code & stop_bit;
        return code & ~stop_bit;
    };

    for (auto& cls : Policy::classes) {
        if (*cls.static_vptr != nullptr) {
            continue;
        }

        indent _1(trace);
        ++trace << "class " << type_name(cls.type) << "\n";

        indent _2(trace);

        auto first_slot = fetch();
        ++trace << "first slot: " << first_slot << "\n";

        *cls.static_vptr = decode_iter - first_slot;

        do {
            auto code = fetch();

            if (code & index_bit) {
                auto index = code & ~index_bit;
                ++trace << "multi-method group " << index << "\n";
                *decode_iter++ = index;
            } else {
                auto method_index = code;
                auto method = methods[method_index];
                auto group_index = fetch(); // spec or group

                if (method->arity() == 1) {
                    ++trace << "uni-method " << method_index << " spec "
                            << group_index;
                    *decode_iter++ = method_defs[method_index][group_index];
                } else {
                    ++trace << "multi-method " << method_index << " group "
                            << group_index;
                    indent _(trace);
                    trace << type_name(method->method_type);
                    *decode_iter++ =
                        (std::uintptr_t)(dispatch_tables[method_index] +
                                         group_index);
                }

                trace << "\n";
                indent _(trace);
                ++trace << type_name(method->method_type) << "\n";
            }
        } while (!last);
    }

    ++trace << decode_iter << " " << encode_iter << "\n";

    auto waste = sizeof(init.encoded) - sizeof(init.vtbls);

    if (waste > 0) {
        ++trace << waste << " bytes wasted\n";
    }

    using namespace policies;

    if constexpr (Policy::template has_facet<policies::external_vptr>) {
        Policy::publish_vptrs(Policy::classes.begin(), Policy::classes.end());
    }
}

} // namespace yomm2
} // namespace yorel

#endif
