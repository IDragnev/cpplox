#include "cpplox/core/Compiler.hpp"
#include <vector>

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

        static const std::vector<ParseRule> table = [&as_index] {
            const auto size = as_index(TokenType::MAX_VALUE);
            std::vector<ParseRule> rules(size);
            // clang-format off
            rules[as_index(TokenType::LEFT_PAREN)] = ParseRule{ .prefix = &Compiler::grouping, };
            rules[as_index(TokenType::MINUS)]      = ParseRule{ .prefix = &Compiler::unary,   .infix = &Compiler::binary, .infixPrec = OpPrecedence::TERM, };
            rules[as_index(TokenType::PLUS)]       = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::TERM, };
            rules[as_index(TokenType::SLASH)]      = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::FACTOR, };
            rules[as_index(TokenType::STAR)]       = ParseRule{                               .infix = &Compiler::binary, .infixPrec = OpPrecedence::FACTOR, };
            rules[as_index(TokenType::NUMBER)]     = ParseRule{ .prefix = &Compiler::number, };
            // clang-format on
            return rules;
        }();

        ParseRule r;
        const std::size_t i = as_index(t);
        if (i < table.size()) {
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

        // temp start
        advance();
        expression();
        emitOpCode(OpCode::RETURN);
        // temp end

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
        errors = std::vector<CompileError>{};
        currentChunk = nullptr;
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
            default: {
                // unreachable
            } break;
        }
    }

    void Compiler::number() {
        Value v = std::strtod(parser.previous.lexeme.data(), nullptr);
        emitConstant(v);
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
        errors.push_back(e);
    }

    void Compiler::emitConstant(Value value) {
        constexpr auto CMAX =
            static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max());
        constexpr auto C16MAX =
            static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max());

        const std::size_t i = addConstant(*currentChunk, value);
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