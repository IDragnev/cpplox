#include "cpplox/core/Scanner.hpp"

namespace cpplox {
    Scanner::Scanner(std::string_view source)
        : source(source)
        , start(source.data())
        , current(source.data()) 
        , line(1)
    {
        toTokenStart();
    }

    void Scanner::toTokenStart() {
        if (isDone() == false) {
            skipWhitespace();
            start = current;
        }
    }

    ScanResult Scanner::scanToken() {
        ScanResult result;
        if (isDone()) {
            result.errorType = ScanError::DONE;
            result.token = makeToken(TokenType::EOF_TOKEN);

            return result;
        }

        result.errorType = ScanError::OK;
        char c = advance();

        if (isDigit(c)) {
            number(result);
            toTokenStart();
            return result;
        }
        if (isAlpha(c)) {
            ident(result);
            toTokenStart();
            return result;
        }

        switch (c) {
            case '(': {
                result.token = makeToken(TokenType::LEFT_PAREN);
            } break;
            case ')': {
                result.token = makeToken(TokenType::RIGHT_PAREN);
            } break;
            case '{': {
                result.token = makeToken(TokenType::LEFT_BRACE);
            } break;
            case '}': {
                result.token = makeToken(TokenType::RIGHT_BRACE);
            } break;
            case ';': {
                result.token = makeToken(TokenType::SEMICOLON);
            } break;
            case ',': {
                result.token = makeToken(TokenType::COMMA);
            } break;
            case '.': {
                result.token = makeToken(TokenType::DOT);
            } break;
            case '-': {
                result.token = makeToken(TokenType::MINUS);
            } break;
            case '+': {
                result.token = makeToken(TokenType::PLUS);
            } break;
            case '/': {
                result.token = makeToken(TokenType::SLASH);
            } break;
            case '*': {
                result.token = makeToken(TokenType::STAR);
            } break;
            case '!': {
                result.token = makeToken(match('=') ? TokenType::BANG_EQUAL
                                                    : TokenType::BANG);
            } break;
            case '=': {
                result.token = makeToken(match('=') ? TokenType::EQUAL_EQUAL
                                                    : TokenType::EQUAL);
            } break;
            case '<': {
                result.token = makeToken(match('=') ? TokenType::LESS_EQUAL
                                                    : TokenType::LESS);
            } break;
            case '>': {
                result.token = makeToken(match('=') ? TokenType::GREATER_EQUAL
                                                    : TokenType::GREATER);
            } break;
            case '"': {
                string(result);
            } break;
            default: {
                scanError(ScanError::UNKNOWN_CHARACTER, result);
            } break;
        }

        toTokenStart();

        return result;
    }

    void Scanner::string(ScanResult& result) {
        while (isDone() == false && peek() != '"') {
            if (peek() == '\n') {
                ++line;
            }

            advance();
        }

        if (isDone() == false) {
            // consume closing "
            advance();

            result.token = makeToken(TokenType::STRING);
        } else {
            scanError(ScanError::UNTERMINATED_STRING, result);
        }
    }

    void Scanner::ident(ScanResult& result) {
        while (isAlpha(peek()) || isDigit(peek())) {
            advance();
        }

        result.token = makeToken(identifierType());
    }

    void Scanner::number(ScanResult& result) {
        while (isDigit(peek())) {
            advance();
        }

        if (peek() == '.' && isDigit(peekNext())) {
            // consume .
            advance();

            while (isDigit(peek())) {
                advance();
            }
        }

        result.token = makeToken(TokenType::NUMBER);
    }

    bool Scanner::isDone() const {
        return *current == '\0';
    }

    char Scanner::peek() const {
        return *current;
    }

    char Scanner::peekNext() const {
        if (isDone()) {
            return '\0';
        }
        return *(current + 1);
    }

    char Scanner::advance() {
        char c = *current;
        ++current;
        return c;
    }

    void Scanner::skipWhitespace() {
        for (;;) {
            switch (peek()) {
            case ' ':
            case '\t':
            case '\r': {
                advance();
            } break;
            case '\n': {
                ++line;
                advance();
            } break;
            case '/': {
                if (peekNext() == '/') {
                    // skip comments
                    while (isDone() == false && peek() != '\n') {
                        advance();
                    }
                }
                else {
                    return;
                }
            } break;
            default: {
                return;
            } break;
            }
        }
    }

    bool Scanner::match(char c) {
        if (isDone()) {
            return false;
        }
        if (*current != c) {
            return false;
        }

        ++current;
        return true;
    }

    Token Scanner::makeToken(TokenType t) const {
        Token token;
        token.type = t;
        token.line = line;
        token.lexeme = std::string_view(start, current);

        return token;
    }

    void Scanner::scanError(ScanError e, ScanResult& r) const {
        r.errorType = e;
        r.token.type = TokenType::ERROR;
        r.token.line = line;
        r.token.lexeme = std::string_view();
    }

    TokenType Scanner::identifierType() const {
        switch (*start) {
            case 'a': return checkKeyword(1, "nd", TokenType::AND);
            case 'c': return checkKeyword(1, "lass", TokenType::CLASS);
            case 'e': return checkKeyword(1, "lse", TokenType::ELSE);
            case 'i': return checkKeyword(1, "f", TokenType::IF);
            case 'n': return checkKeyword(1, "il", TokenType::NIL);
            case 'o': return checkKeyword(1, "r", TokenType::OR);
            case 'p': return checkKeyword(1, "rint", TokenType::PRINT);
            case 'r': return checkKeyword(1, "eturn", TokenType::RETURN);
            case 's': return checkKeyword(1, "uper", TokenType::SUPER);
            case 'v': return checkKeyword(1, "ar", TokenType::VAR);
            case 'w': return checkKeyword(1, "hile", TokenType::WHILE);
            case 'f': {
                if (current - start > 1) {
                    switch (*(start + 1)) {
                        case 'a': return checkKeyword(2, "lse", TokenType::FALSE);
                        case 'o': return checkKeyword(2, "r", TokenType::FOR);
                        case 'u': return checkKeyword(2, "n", TokenType::FUN);
                    }
                }
            } break;
            case 't': {
                if (current - start > 1) {
                    switch (*(start + 1)) {
                        case 'h': return checkKeyword(2, "is", TokenType::THIS);
                        case 'r': return checkKeyword(2, "ue", TokenType::TRUE);
                    }
                }
            } break;
        }

        return TokenType::IDENTIFIER;
    }

    TokenType Scanner::checkKeyword(unsigned matchedLen,
                                    std::string_view rest,
                                    TokenType expectedType) const {
        const auto lexemeLen = static_cast<int>(current - start);
        const auto expetedLen = static_cast<int>(matchedLen + rest.size());
        if (lexemeLen == expetedLen) {
            const auto suffix = std::string_view(start + matchedLen, current);
            if (suffix == rest) {
                return expectedType;
            }
        }

        return TokenType::IDENTIFIER;
    }

    bool Scanner::isDigit(char c) const {
        return c >= '0' && c <= '9';
    }

    bool Scanner::isAlpha(char c) const {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c == '_');
    }
} // namespace cpplox