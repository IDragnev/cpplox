#pragma once

#include "cpplox/compiler/Scanner.hpp"
#include "cpplox/compiler/Chunk.hpp"

namespace cpplox {
    class DiagnosticEngine;

    class Compiler {
    private:
        struct ParseRule;
        enum class OpPrecedence;

    public:
        Compiler(DiagnosticEngine* engine);
        ~Compiler() = default;

        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;

        bool compile(std::string source, Chunk& chunk);

    private:
        void init(std::string&& source, Chunk& chunk);
        void cleanUp();

        void synchronize();
        void advance();
        bool consumeToken(TokenType token);
        bool match(TokenType type);

        template <typename... Args>
        void consumeTokenErr(TokenType token, std::string_view fmt, Args&&... args);
        template <typename... Args>
        void compileError(const Token& t, std::string_view fmt, Args&&... args);
        bool processError();

        bool makeConstant(Value value,
                          bool searchExisting,
                          std::size_t& idx,
                          bool& largeIdx);
        void emitConstant(Value value);
        void emitConstantInstruction(OpCode small,
                                     OpCode big,
                                     std::size_t idx,
                                     bool const16);
        void emitOpCode(OpCode op);
        void emitByte(std::uint8_t byte);
        void emitBytes(std::uint8_t a, std::uint8_t b);

        // statement parsers
        void declaration();
        void varDeclaration();
        void parseVariable(std::size_t& idx, bool& largeIdx);
        void defineVariable(std::size_t idx, bool largeIdx);
        void statement();
        void printStatement();
        void expressionStatement();

        // expression parsers
        static ParseRule getParseRule(TokenType t);
        void parsePrecedence(OpPrecedence p);
        void expression();
        void variable(bool canAssign);
        void namedVariable(const Token& t, bool canAssign);
        void number(bool canAssign);
        void grouping(bool canAssign);
        void unary(bool canAssign);
        void binary(bool canAssign);
        void literal(bool canAssign);
        void string(bool canAssign);

    private:
        std::string source = "";
        Scanner scanner;
        struct Parser {
            Token previous;
            Token current;
            bool hadError = false;
            bool panicMode = false;
        } parser;
        Chunk* currentChunk = nullptr;
        DiagnosticEngine* diagnostics = nullptr;
    };
} // namespace cpplox