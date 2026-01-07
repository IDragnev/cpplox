#pragma once

#include "cpplox/compiler/Chunk.hpp"
#include "cpplox/compiler/Scanner.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class DiagnosticEngine;
    enum class OpCode;

    class Compiler {
    private:
        struct ParseRule;
        enum class OpPrecedence;

        struct Local {
            Token name;
            std::uint16_t depth = 0;
            bool initialized = false;
        };

    public:
        Compiler(DiagnosticEngine* engine);
        ~Compiler() = default;

        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;

        bool compile(std::string source, Chunk& chunk);

    private:
        void init(std::string&& source, Chunk& chunk);
        void cleanUp();

        void beginScope();
        void endScope();
        void addLocal(const Token& name);
        bool resolveLocal(const Token& name, std::size_t& idx);

        void synchronize();
        void advance();
        bool consumeToken(TokenType token);
        bool match(TokenType type);
        bool peek(TokenType type) const;

        template <typename... Args>
        void consumeTokenErr(TokenType token, std::string_view fmt, Args&&... args);
        template <typename... Args>
        void compileError(const Token& t, std::string_view fmt, Args&&... args);
        bool processError();

        bool makeConstant(Value value,
                          bool searchExisting,
                          std::size_t& idx);
        void emitConstant(Value value);
        void emitIntegerInstruction(OpCode small,
                                    OpCode big,
                                    std::size_t operand);
        void emitOpCode(OpCode op);
        void emitByte(std::uint8_t byte);
        void emitBytes(std::uint8_t a, std::uint8_t b);

        // statement parsers
        void declaration();
        void varDeclaration();
        void parseVariable(std::size_t& idx);
        void declareVariable(const Token& t);
        void defineVariable(std::size_t idx);
        void statement();
        void block();
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
        Vector<Local> locals;
        std::uint16_t scopeDepth = 0;
        DiagnosticEngine* diagnostics = nullptr;
    };
} // namespace cpplox