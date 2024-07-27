#ifndef YOREL_YOMM2_DETAIL_UPDATE_METHOD_REPORT_HPP
#define YOREL_YOMM2_DETAIL_UPDATE_METHOD_REPORT_HPP

namespace yorel {
namespace yomm2 {
namespace detail {

struct update_method_report {
    size_t cells = 0;
    size_t concrete_cells = 0;
    size_t not_implemented = 0;
    size_t concrete_not_implemented = 0;
    size_t ambiguous = 0;
    size_t concrete_ambiguous = 0;
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
