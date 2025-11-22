#include "cpplox/core/String.hpp"

#include <cstring>
#include <algorithm>

namespace cpplox {
    String::String(char c) : content{new char[2]{c, '\0'}}, len(1) {}

    String::String(const String& other) : String(other.content) {}

    String::String(String&& source) noexcept
        : content{source.content}
        , len(source.len)
    {
        source.content = nullptr;
        source.len = 0;
    }

    String::String(const char* string) {
        if (string != nullptr) {
            len = std::strlen(string);
            content = new char[len + 1] {};
            std::copy_n(string, len, content);
        }
    }

    String::String(std::string_view string) {
        if (string.size() > 0) {
            len = string.size();
            content = new char[len + 1]{};
            std::copy_n(string.data(), len, content);
        }
    }

    String& String::operator=(const String& rhs) {
        if (this != &rhs) {
            String copy(rhs);
            std::swap(copy.content, this->content);
        }

        return *this;
    }

    String& String::operator=(String&& rhs) noexcept {
        if (this != &rhs) {
            String temp = std::move(rhs);
            std::swap(content, temp.content);
        }

        return *this;
    }

    String::~String() {
        delete[] content;
    }

    void String::append(const char* string) {
        if (string == nullptr) {
            return;
        }

        if (std::size_t sourceLen = std::strlen(string); sourceLen > 0) {
            std::size_t currentLen = size();
            auto buffer = new char[sourceLen + currentLen + 1];

            std::copy_n(this->content, currentLen, buffer);
            std::copy_n(string, sourceLen + 1, buffer + currentLen);

            delete[] this->content;
            this->content = buffer;
        }
    }

    const char* String::c_str() const {
        return (content != nullptr) ? content : "";
    }

    std::size_t String::size() const {
        return len;
    }

    String& String::operator+=(const String& string) {
        append(string.c_str());

        return *this;
    }

    String& String::operator+=(const char* string) {
        append(string);

        return *this;
    }

    String& String::operator+=(char c) {
        char buffer[]{c, '\0'};
        append(buffer);

        return *this;
    }

    bool operator==(const String& lhs, const String& rhs) {
        return std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
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
