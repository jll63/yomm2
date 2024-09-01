#ifndef YOREL_YOMM2_DETAIL_LIST_INCLUDED
#define YOREL_YOMM2_DETAIL_LIST_INCLUDED

#include <algorithm>
#include <boost/assert.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

template<typename T>
class static_list {
  public:
    static_list(static_list&) = delete;
    static_list() = default;

    class static_link {
      public:
        static_link(const static_link&) = delete;
        static_link() = default;

        T* next() {
            return next_ptr;
        }

      protected:
        friend class static_list;
        T* prev_ptr;
        T* next_ptr;
    };

    void push_back(T& node) {
        BOOST_ASSERT(node.prev_ptr == nullptr);
        BOOST_ASSERT(node.next_ptr == nullptr);

        if (!first) {
            first = &node;
            node.prev_ptr = &node;
            return;
        }

        auto last = first->prev_ptr;
        last->next_ptr = &node;
        node.prev_ptr = last;
        first->prev_ptr = &node;
    }

    void remove(T& node) {
        BOOST_ASSERT(first != nullptr);

        auto prev = node.prev_ptr;
        auto next = node.next_ptr;
        auto last = first->prev_ptr;

        node.prev_ptr = nullptr;
        node.next_ptr = nullptr;

        if (&node == last) {
            if (&node == first) {
                first = nullptr;
                return;
            }

            first->prev_ptr = prev;
            prev->next_ptr = nullptr;
            return;
        }

        if (&node == first) {
            first = next;
            first->prev_ptr = last;
            return;
        }

        prev->next_ptr = next;
        next->prev_ptr = prev;
    }

    void clear() {
        auto next = first;
        first = nullptr;

        while (next) {
            auto cur = next;
            next = cur->next_ptr;
            cur->prev_ptr = nullptr;
            cur->next_ptr = nullptr;
        }
    }

    class iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() : ptr(nullptr) {
        }
        explicit iterator(T* p) : ptr(p) {
        }

        reference operator*() {
            return *ptr;
        }
        pointer operator->() {
            return ptr;
        }

        iterator& operator++() {
            BOOST_ASSERT(ptr);
            ptr = ptr->next_ptr;
            return *this;
        }

        iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.ptr == b.ptr;
        };
        friend bool operator!=(const iterator& a, const iterator& b) {
            return a.ptr != b.ptr;
        };

      private:
        T* ptr;
    };

    iterator begin() {
        return iterator(first);
    }

    iterator end() {
        return iterator(nullptr);
    }

    class const_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        const_iterator() : ptr(nullptr) {
        }
        explicit const_iterator(T* p) : ptr(p) {
        }

        reference operator*() {
            return *ptr;
        }
        pointer operator->() {
            return ptr;
        }

        const_iterator& operator++() {
            BOOST_ASSERT(ptr);
            ptr = ptr->next_ptr;
            return *this;
        }

        const_iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool
        operator==(const const_iterator& a, const const_iterator& b) {
            return a.ptr == b.ptr;
        };
        friend bool
        operator!=(const const_iterator& a, const const_iterator& b) {
            return a.ptr != b.ptr;
        };

      private:
        T* ptr;
    };

    const_iterator begin() const {
        return const_iterator(first);
    }

    const_iterator end() const {
        return const_iterator(nullptr);
    }

    std::size_t size() const {
        return std::distance(begin(), end());
    }

    bool empty() const {
        return !first;
    }

  protected:
    T* first;
};

} // namespace detail
} // namespace yomm2
} // namespace yorel
#endif
