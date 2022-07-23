#ifndef YOREL_YOMM2_CHAIN_INCLUDED
#define YOREL_YOMM2_CHAIN_INCLUDED

#include <algorithm> // IWYU pragma: keep
#include <cassert>
#include <iterator> // IWYU pragma: keep

template<typename T>
class static_chain {
  public:
    static_chain(static_chain&) = delete;
    static_chain() = default;

    explicit static_chain(int) : first(nullptr), removed_prev(nullptr) {}

    class static_link {
      public:
        static_link(const static_link&) = delete;
        static_link() = default;
        explicit static_link(int) : _next(nullptr) {}

        T* next() { return _next; }

      protected:
        friend class static_chain;
        T* _next;
    };

    struct link : static_link {
        link() : static_link(0) {}
    };

    void push_front(T& node) {
        assert(node._next == nullptr);
        node._next = first;
        first = &node;
    }

    void remove(T& node) {
        iterator iter;

        if (&node == first) {
            first = node._next;
            node._next = nullptr;
            removed_prev = nullptr;
            return;
        }

        if (removed_prev != nullptr && removed_prev != &node) {
            iter = std::find_if(
                iterator(removed_prev), end(),
                [&node](T& other) { return other._next == &node; });
            if (iter == end()) {
                iter = std::find_if(
                    begin(), iterator(removed_prev),
                    [&node](T& other) { return other._next == &node; });
            }
        } else {
            iter = std::find_if(begin(), end(), [&node](T& other) {
                return other._next == &node;
            });
        }

        if (iter == end()) {
            assert(false);
            abort();
        }

        iter->_next = node._next;
        removed_prev = &*iter;
    }

    class iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() : ptr(nullptr) {}
        explicit iterator(T* p) : ptr(p) {}

        reference operator*() { return *ptr; }
        pointer operator->() { return ptr; }

        iterator& operator++() {
            assert(ptr);
            ptr = ptr->_next;
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

    iterator begin() { return iterator(first); }

    iterator end() { return iterator(nullptr); }

    class const_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        const_iterator() : ptr(nullptr) {}
        explicit const_iterator(T* p) : ptr(p) {}

        reference operator*() { return *ptr; }
        pointer operator->() { return ptr; }

        const_iterator& operator++() {
            assert(ptr);
            ptr = ptr->_next;
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

    const_iterator begin() const { return const_iterator(first); }

    const_iterator end() const { return const_iterator(nullptr); }

    size_t size() const { return std::distance(begin(), end()); }

  protected:
    T* first;
    T* removed_prev;
};

template<typename T>
class chain : public static_chain<T> {
  public:
    chain() {
        this->first = nullptr;
        this->removed_prev = nullptr;
    }
};

#endif
