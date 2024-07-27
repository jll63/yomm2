#ifndef YOREL_YOMM2_DETAIL_RANGE_HPP
#define YOREL_YOMM2_DETAIL_RANGE_HPP

namespace yorel {
namespace yomm2 {
namespace detail {

template<typename Iterator>
struct range {
    range(Iterator first, Iterator last) : first(first), last(last) {
    }

    Iterator first, last;

    Iterator begin() const {
        return first;
    }

    Iterator end() const {
        return last;
    }
};

template<typename Iterator>
range(Iterator b, Iterator e) -> range<Iterator>;

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif // YOREL_YOMM2_DETAIL_RANGE_HPP
