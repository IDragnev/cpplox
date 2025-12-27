#include "cpplox/core/Compiler.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    enum class Compiler::OpPrecedence {
        NONE,
        ASSIGNMENT, // =
        OR,         // or
        AND,        // and
        EQUALITY,   // == !=
        COMPARISON, // < > <= >=
        TERM,       // + -
        FACTOR,     // * /
        UNARY,      // ! -
        CALL,       // . ()
        PRIMARY
    };

    using ParseFn = void (Compiler::*)();

    struct Compiler::ParseRule {
        ParseFn prefix = nullptr;
        ParseFn infix = nullptr;
        OpPrecedence infixPrec = OpPrecedence::NONE;
    };

    Compiler::ParseRule Compiler::getParseRule(TokenType t) {
        const auto as_index = [](TokenType t) {
            return static_cast<std::size_t>(t);
        };

        static const Vector<ParseRule> table = [&as_index] {
            const auto size = as_index(TokenType::MAX_VALUE);
            Vector<ParseRule> rules(size);
            // clang-format off
            rules[as_index(TokenType::LEFT_PAREN)]    = ParseRule{ .prefix = &Compiler::grouping, };
            rules[as_index(TokenType::MINUS)]         = ParseRule{ .prefix = &Compiler::unary,   .infix = &Compiler::binary, .infixPrec = OpPrecedence::TERM, };
            rules[as_index(TokenType::PLUS)]          = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::TERM, };
            rules[as_index(TokenType::SLASH)]         = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::FACTOR, };
            rules[as_index(TokenType::STAR)]          = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::FACTOR, };
            rules[as_index(TokenType::EQUAL_EQUAL)]   = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::EQUALITY, };
            rules[as_index(TokenType::BANG_EQUAL)]    = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::EQUALITY, };
            rules[as_index(TokenType::GREATER_EQUAL)] = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::COMPARISON, };
            rules[as_index(TokenType::GREATER)]       = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::COMPARISON, };
            rules[as_index(TokenType::LESS_EQUAL)]    = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::COMPARISON, };
            rules[as_index(TokenType::LESS)]          = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::COMPARISON, };
            rules[as_index(TokenType::NUMBER)]        = ParseRule{ .prefix = &Compiler::number, };
            rules[as_index(TokenType::TRUE)]          = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::FALSE)]         = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::NIL)]           = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::BANG)]          = ParseRule{ .prefix = &Compiler::unary, };
            rules[as_index(TokenType::STRING)]        = ParseRule{ .prefix = &Compiler::string, };
            // clang-format on
            return rules;
        }();

        ParseRule r;
        const std::size_t i = as_index(t);
        if (i < table.getCount()) {
            r = table[i];
        }

        return r;
    }

    Compiler::Compiler()
        : source("")
        , scanner(source)
    {
    }

    CompileResult Compiler::compile(std::string compileSource, Chunk& chunk) {
        init(std::move(compileSource), chunk);

        advance();
        while (scanner.isDone() == false) {
            declaration();

            if (parser.panicMode) {
                synchronize();
            }
        }

        CompileResult result;
        result.errors = std::move(this->errors);
        result.hasError = parser.hadError;

        cleanUp();

        return result;
    }

    void Compiler::init(std::string&& compileSource, Chunk& chunk) {
        source = std::move(compileSource);
        scanner = Scanner(source);
        currentChunk = &chunk;
    }

    void Compiler::cleanUp() {
        source = "";
        scanner = Scanner(source);
        parser = Parser{};
        errors = Vector<CompileError>{};
        currentChunk = nullptr;
    }

    void Compiler::synchronize() {
        parser.panicMode = false;

        while (scanner.isDone() == false) {
            if (parser.previous.type == TokenType::SEMICOLON) {
                return;
            }
            switch (parser.current.type) {
                case TokenType::CLASS:
                case TokenType::FUN:
                case TokenType::VAR:
                case TokenType::FOR:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::PRINT:
                case TokenType::RETURN: {
                    return;
                } break;
                default: { }
            }

            advance();
        }
    }

    void Compiler::advance() {
        parser.previous = parser.current;

        while (scanner.isDone() == false) {
            ScanResult r = scanner.scanToken();
            parser.current = r.token;

            if (r.errorType == ScanError::OK) {
                break;
            }
            else {
                addError(CompileError{
                    .type = CompileErrorType::SCAN_ERROR,
                    .scanError = r.errorType,
                    .token = parser.current,
                });
            }
        }
    }

    void Compiler::declaration() {
        statement();
    }

    void Compiler::statement() {
        if (match(TokenType::PRINT)) {
            printStatement();
        } else {
            expressionStatement();
        }
    }

    void Compiler::printStatement() {
        expression();
        consumeToken(TokenType::SEMICOLON);
        emitOpCode(OpCode::PRINT);
    }

    void Compiler::expressionStatement() {
        expression();
        consumeToken(TokenType::SEMICOLON);
        emitOpCode(OpCode::POP);
    }

    void Compiler::expression() {
        parsePrecedence(OpPrecedence::ASSIGNMENT);
    }

    void Compiler::parsePrecedence(OpPrecedence precedence) {
        advance();

        ParseRule rule = getParseRule(parser.previous.type);
        ParseFn prefixRule = rule.prefix;
        if (prefixRule == nullptr) {
            addError(CompileError {
                .type = CompileErrorType::EXPECTED_EXPRESSION,
                .token = parser.previous,
            });
            return;
        }

        (this->*prefixRule)();

        while (precedence <= getParseRule(parser.current.type).infixPrec) {
            advance();
            ParseRule r = getParseRule(parser.previous.type);
            ParseFn infixRule = r.infix;
            if (infixRule != nullptr) {
                (this->*infixRule)();
            }
        }
    }

    void Compiler::grouping() {
        expression();
        consumeToken(TokenType::RIGHT_PAREN);
    }

    void Compiler::binary() {
        TokenType op = parser.previous.type;
        ParseRule rule = getParseRule(op);
        parsePrecedence(static_cast<OpPrecedence>(static_cast<int>(rule.infixPrec) + 1));

        switch (op) {
            case TokenType::PLUS: {
                emitOpCode(OpCode::ADD);
            } break;
            case TokenType::MINUS: {
                emitOpCode(OpCode::SUBTRACT);
            } break;
            case TokenType::STAR: {
                emitOpCode(OpCode::MULTIPLY);
            } break;
            case TokenType::SLASH: {
                emitOpCode(OpCode::DIVIDE);
            } break;
            case TokenType::EQUAL_EQUAL: {
                emitOpCode(OpCode::EQUAL);
            } break;
            case TokenType::BANG_EQUAL: {
                emitOpCode(OpCode::NOT_EQUAL);
            } break;
            case TokenType::GREATER: {
                emitOpCode(OpCode::GREATER);
            } break;
            case TokenType::GREATER_EQUAL: {
                emitOpCode(OpCode::GREATER_EQUAL);
            } break;
            case TokenType::LESS: {
                emitOpCode(OpCode::LESS);
            } break;
            case TokenType::LESS_EQUAL: {
                emitOpCode(OpCode::LESS_EQUAL);
            } break;
            default: {
                // unreachable
            } break;
        }
    }

    void Compiler::literal() {
        switch (parser.previous.type) {
            case TokenType::TRUE: {
                emitOpCode(OpCode::TRUE);
            } break;
            case TokenType::FALSE: {
                emitOpCode(OpCode::FALSE);
            } break;
            case TokenType::NIL: {
                emitOpCode(OpCode::NIL);
            } break;
            default: {
                // unreachable
            } break;
        }
    }

    void Compiler::unary() {
        TokenType type = parser.previous.type;
        parsePrecedence(OpPrecedence::UNARY);

        switch (type) {
            case TokenType::MINUS: {
                emitOpCode(OpCode::NEGATE);
            } break;
            case TokenType::BANG: {
                emitOpCode(OpCode::NOT);
            } break;
            default: {
                // unreachable
            } break;
        }
    }

    void Compiler::number() {
        double v = std::strtod(parser.previous.lexeme.data(), nullptr);
        emitConstant(Value(v));
    }

    void Compiler::string() {
        std::string_view v = parser.previous.lexeme;
        // trim quotes
        v.remove_prefix(1);
        v.remove_suffix(1);

        emitConstant(Value(v));
    }

    bool Compiler::match(TokenType type) {
        if (parser.current.type == type) {
            advance();
            return true;
        }

        return false;
    }

    bool Compiler::consumeToken(TokenType tokenType) {
        if (parser.current.type == tokenType) {
            advance();
            return true;
        }

        addError(CompileError{
            .type = CompileErrorType::EXPECTED_TOKEN,
            .token = parser.current,
            .expectedToken = tokenType,
        });

        return false;
    }

    void Compiler::addError(const CompileError& e) {
        if (parser.panicMode) {
            return;
        }

        parser.panicMode = true;
        parser.hadError = true;
        errors.insertBack(e);
    }

    void Compiler::emitConstant(Value value) {
        constexpr auto CMAX =
            static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max());
        constexpr auto C16MAX =
            static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max());

        const std::size_t i = addConstant(*currentChunk, std::move(value));
        if (i <= CMAX) {
            emitOpCode(OpCode::CONSTANT);
            emitByte(static_cast<std::uint8_t>(i));
        } else if (i <= C16MAX) {
            std::uint8_t a = 0;
            std::uint8_t b = 0;
            serializeConstant16Index(i, a, b);

            emitOpCode(OpCode::CONSTANT_16);
            emitBytes(a, b);
        } else {
            addError(CompileError{
                .type = CompileErrorType::CONSTANTS_LIMIT_REACHED,
                .token = parser.previous,
            });
        }
    }

    void Compiler::emitOpCode(OpCode op) {
        emitByte(static_cast<std::uint8_t>(op));
    }

    void Compiler::emitByte(std::uint8_t byte) {
        addCode(*currentChunk, byte, parser.previous.line);
    }

    void Compiler::emitBytes(std::uint8_t a, std::uint8_t b) {
        emitByte(a);
        emitByte(b);
    }
}