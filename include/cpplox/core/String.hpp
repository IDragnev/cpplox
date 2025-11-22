#pragma once

#include <cstddef>
#include <string_view>

namespace cpplox {
    class String {
    public:
        String() = default;
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

        String& operator+=(const String& rhs);
        String& operator+=(const char* rhs);
        String& operator+=(char rhs);

    private:
        void append(const char* string);

    private:
        char* content = nullptr;
        std::size_t len = 0;
    };

    String operator+(const String& lhs, char rhs);
    String operator+(char lhs, const String& rhs);
    String operator+(const String& lhs, const String& rhs);

    bool operator==(const String& lhs, const String& rhs);
    bool operator!=(const String& lhs, const String& rhs);
} // namespace cpplox