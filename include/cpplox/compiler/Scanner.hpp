#pragma once

#include "cpplox/compiler/Token.hpp"

#include <string>
#include <string_view>

namespace cpplox {
    enum class ScanError {
        OK,
        DONE,
        UNTERMINATED_STRING,
        UNKNOWN_CHARACTER,
    };

    struct ScanResult {
        ScanError errorType = ScanError::OK;
        Token token;
    };

    class Scanner {
    public:
        Scanner(std::string_view source);

        bool isDone() const;
        ScanResult scanToken();

    private:
        void toTokenStart();

        char peek() const;
        char peekNext() const;
        char advance();
        bool match(char c);
        void skipWhitespace();

        void string(ScanResult& result);
        void number(ScanResult& result);
        void ident(ScanResult& result);
        void scanError(ScanError e, ScanResult& r) const;
        Token makeToken(TokenType t) const;

        TokenType identifierType() const;
        TokenType checkKeyword(unsigned matchedLen,
                               std::string_view rest,
                               TokenType expectedType) const;

        bool isDigit(char c) const;
        bool isAlpha(char c) const;

    private:
        std::string_view source;
        const char* start = nullptr;
        const char* current = nullptr;
        unsigned line = 0;
    };
} // namespace cpplox