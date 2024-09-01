#ifndef YOREL_YOMM2_DETAIL_OSTDSTREAM_HPP
#define YOREL_YOMM2_DETAIL_OSTDSTREAM_HPP

#include <array>
#include <cstdio>

namespace yorel {
namespace yomm2 {
namespace detail {

// -----------------------------------------------------------------------------
// lightweight ostream

struct ostdstream {
    FILE* stream = nullptr;

    ostdstream(FILE* stream = nullptr) : stream(stream) {
    }

    void on(FILE* stream = stderr) {
        this->stream = stream;
    }

    void off() {
        this->stream = nullptr;
    }

    bool is_on() const {
        return stream != nullptr;
    }
};

struct ostderr : ostdstream {
    ostderr() : ostdstream(stderr) {
    }
};

inline ostdstream cerr;

inline ostdstream& operator<<(ostdstream& os, const char* str) {
    if (os.stream) {
        fputs(str, os.stream);
    }

    return os;
}

inline ostdstream& operator<<(ostdstream& os, const std::string_view& view) {
    if (os.stream) {
        fwrite(view.data(), sizeof(*view.data()), view.length(), os.stream);
    }

    return os;
}

inline ostdstream& operator<<(ostdstream& os, const void* value) {
    if (os.stream) {
        std::array<char, 20> str;
        auto end = std::to_chars(
                       str.data(), str.data() + str.size(),
                       reinterpret_cast<uintptr_t>(value), 16)
                       .ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

inline ostdstream& operator<<(ostdstream& os, std::size_t value) {
    if (os.stream) {
        std::array<char, 20> str;
        auto end =
            std::to_chars(str.data(), str.data() + str.size(), value).ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
