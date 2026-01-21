#pragma once

#include <cstddef>
#include <string_view>

namespace cpplox {
    class String {
    public:
        String();
        String(char symbol);
        String(const char* string);
        String(std::string_view string);
        String(const String& source);
        String(String&& source) noexcept;
        ~String();

        String& operator=(String&& rhs) noexcept;
        String& operator=(const String& rhs);

        const char* c_str() const;
        std::size_t size() const;
        std::size_t capacity() const;
        std::uint32_t hashValue() const;

        void reserve(std::size_t capacity);

        String& operator+=(const String& rhs);
        String& operator+=(const char* rhs);
        String& operator+=(std::string_view sv);
        String& operator+=(char rhs);

    private:
        void append(const char* string, std::size_t len);
        void updateHash();

    private:
        char* content = nullptr;
        std::size_t len = 0;
        std::size_t cap = 0;
        std::uint32_t hash = 0;
    };

    String operator+(const String& lhs, char rhs);
    String operator+(char lhs, const String& rhs);
    String operator+(const String& lhs, const String& rhs);

    bool operator==(const String& lhs, const String& rhs);
    bool operator!=(const String& lhs, const String& rhs);
} // namespace cpplox