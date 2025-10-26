#pragma once

#include "cpplox/core/Scanner.hpp"
#include "cpplox/core/Chunk.hpp"

#include <vector>

namespace cpplox {
    enum class CompileErrorType {
        UNKNOWN,
        SCAN_ERROR,
        EXPECTED_TOKEN,
        EXPECTED_EXPRESSION,
        EXPECTED_STATEMENT,
        CONSTANTS_LIMIT_REACHED,
    };

    struct CompileError {
        CompileErrorType type = CompileErrorType::UNKNOWN;
        ScanError scanError = ScanError::OK;
        Token token;
        TokenType expectedToken = TokenType::EOF_TOKEN;
    };

    struct CompileResult {
        bool hasError = false;
        std::vector<CompileError> errors;
    };

    class Compiler {
    private:
        struct ParseRule;
        enum class OpPrecedence;

    public:
        Compiler();
        ~Compiler() = default;

        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;

        CompileResult compile(std::string source, Chunk& chunk);

    private:
        void init(std::string&& source, Chunk& chunk);
        void cleanUp();

        void advance();
        void addError(const CompileError& e);
        bool consumeToken(TokenType token);

        void emitConstant(Value value);
        void emitOpCode(OpCode op);
        void emitByte(std::uint8_t byte);
        void emitBytes(std::uint8_t a, std::uint8_t b);

        // expression parsers
        static ParseRule getParseRule(TokenType t);
        void parsePrecedence(OpPrecedence p);
        void expression();
        void number();
        void grouping();
        void unary();
        void binary();

    private:
        std::string source = "";
        Scanner scanner;
        struct Parser {
            Token previous;
            Token current;
            bool hadError = false;
            bool panicMode = false;
        } parser;
        std::vector<CompileError> errors;
        Chunk* currentChunk = nullptr;
    };
} // namespace cpplox