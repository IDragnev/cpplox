#include "cpplox/core/String.hpp"

#include <cstring>
#include <algorithm>

namespace cpplox {
    String::String() { updateHash(); }

    String::String(char c)
        : content{new char[2]{c, '\0'}}
        , len(1)
        , cap(2)
    {
        updateHash();
    }

    String::String(const String& other) : String(other.content) {}

    String::String(String&& source) noexcept
        : content{source.content}
        , len(source.len)
        , cap(source.cap)
        , hash(source.hash)
    {
        source.content = nullptr;
        source.len = 0;
        source.cap = 0;
        source.updateHash();
    }

    String::String(const char* string) {
        if (string != nullptr) {
            len = std::strlen(string);
            cap = len + 1;
            content = new char[cap] {};
            std::copy_n(string, len, content);
        }
        updateHash();
    }

    String::String(std::string_view string) {
        if (string.size() > 0) {
            len = string.size();
            cap = len + 1;
            content = new char[cap]{};
            std::copy_n(string.data(), len, content);
        }
        updateHash();
    }

    String& String::operator=(const String& rhs) {
        if (this != &rhs) {
            String copy(rhs);
            std::swap(copy.len, this->len);
            std::swap(copy.cap, this->cap);
            std::swap(copy.content, this->content);
            std::swap(copy.hash, this->hash);
        }

        return *this;
    }

    String& String::operator=(String&& rhs) noexcept {
        if (this != &rhs) {
            String temp = std::move(rhs);
            std::swap(len, temp.len);
            std::swap(cap, temp.cap);
            std::swap(content, temp.content);
            std::swap(hash, temp.hash);
        }

        return *this;
    }

    String::~String() {
        delete[] content;
    }

    void String::append(const char* string, std::size_t sourceLen) {
        if (string == nullptr) {
            return;
        }
        if (sourceLen == 0) {
            return;
        }

        const std::size_t neededCap = len + sourceLen + 1;
        if (neededCap > cap) {
            reserve(neededCap);
        }

        std::copy_n(string, sourceLen, content + len);
        len += sourceLen;

        updateHash();
    }

    void String::reserve(std::size_t capacity) {
        if (cap >= capacity) {
            return;
        }

        auto buffer = new char[capacity]{};
        if (len > 0) {
            std::copy_n(content, len, buffer);
        }

        delete[] content;
        content = buffer;
        cap = capacity;
    }

    void String::updateHash() {
        // FNV-1a
        hash = 2166136261u;
        for (std::size_t i = 0; i < len; i++) {
            hash ^= static_cast<std::uint8_t>(content[i]);
            hash *= 16777619;
        }
    }

    const char* String::c_str() const {
        return (content != nullptr) ? content : "";
    }

    std::size_t String::size() const {
        return len;
    }

    std::size_t String::capacity() const {
        return cap;
    }

    std::uint32_t String::hashValue() const {
        return hash;
    }

    String& String::operator+=(const String& string) {
        append(string.c_str(), string.size());

        return *this;
    }

    String& String::operator+=(const char* string) {
        append(string, std::strlen(string));

        return *this;
    }

    String& cpplox::String::operator+=(std::string_view sv) {
        append(sv.data(), sv.size());

        return *this;
    }

    String& String::operator+=(char c) {
        char buffer[]{c, '\0'};
        append(buffer, 2);

        return *this;
    }

    bool operator==(const String& lhs, const String& rhs) {
        return lhs.size() == rhs.size() &&
               lhs.hashValue() == rhs.hashValue() &&
               std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    bool operator!=(const String& lhs, const String& rhs) {
        return !(lhs == rhs);
    }

    String operator+(const String& lhs, const String& rhs) {
        auto result = lhs;
        result += rhs;

        return result;
    }

    String operator+(char lhs, const String& rhs) {
        auto result = String{lhs};
        result += rhs;

        return result;
    }

    String operator+(const String& lhs, char rhs) {
        auto result = lhs;
        result += rhs;

        return result;
    }
} // namespace cpplox
