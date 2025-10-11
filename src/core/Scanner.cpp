#include "cpplox/core/Scanner.hpp"

namespace cpplox {
    Scanner::Scanner(std::string_view source)
        : source(source)
        , start(source.data())
        , current(source.data()) 
        , line(1)
    {
    }

    ScanResult Scanner::scanToken() {
        skipWhitespace();

        start = current;

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

        return result;
    }

    void Scanner::string(ScanResult& result) {
        // consume leading "
        advance();

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
                    } else {
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

    bool Scanner::isDigit(char c) const {
        return c >= '0' && c <= '9';
    }
} // namespace cpplox