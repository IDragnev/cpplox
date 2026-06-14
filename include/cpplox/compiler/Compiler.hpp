#pragma once

#include "cpplox/compiler/Scanner.hpp"
#include "cpplox/core/Vector.hpp"
#include "cpplox/core/Value.hpp"

#include <string>

namespace cpplox {
    class Function;
    class Object;
    class DiagnosticEngine;
    enum class OpCode;
    class String;

    struct CompileResult {
        bool error = false;
        Function* function = nullptr;
        Vector<Object*> gcObjects;
    };

    class Compiler {
    private:
        struct ParseRule;
        struct Frame;
        enum class OpPrecedence;

        enum class FunctionType {
            SCRIPT,
            FUNCTION,
            METHOD,
            INITIALIZER,
        };

        struct Local {
            Token name;
            std::uint16_t depth = 0;
            bool initialized = false;
            bool captured = false;
        };

        struct Upvalue {
            std::size_t index = 0;
            bool isLocal = false;

            bool operator==(const Upvalue& v) const {
                return index == v.index && isLocal == v.isLocal;
            }
        };

    public:
        Compiler();
        ~Compiler() = default;

        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;

        CompileResult compile(std::string src, DiagnosticEngine* engine);
        CompileResult replExpression(std::string src, DiagnosticEngine* engine);

    private:
        bool init(std::string&& source, DiagnosticEngine* engine);
        void initFrame(Frame& frame, Function* f, FunctionType t, Frame* parent);
        CompileResult prepareResult(bool hadError);
        void cleanUp();
        template <typename T, typename... Args>
        T* makeObject(Args&&... args);

        bool inLocalScope() const;
        void beginScope();
        void endScope();
        void addLocal(const Token& name);
        void addUpvalue(Frame& frame,
                        const Token& name,
                        Upvalue upv,
                        std::size_t& upvalueIdx);
        bool resolveLocal(Frame& frame, const Token& name, std::size_t& idx);
        bool resolveUpvalue(Frame& frame, const Token& name, std::size_t& idx);

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
        void emitReturn();
        void emitConstant(Value value);
        void emitClosure(Function* fun, const Frame& closureFrame);
        void emitIntegerInstruction(OpCode small,
                                    OpCode big,
                                    std::size_t operand);
        void emitTwoByteIntegerInstruction(OpCode op, std::size_t operand);
        void emitLoop(std::size_t loopStart);
        std::size_t emitJump(OpCode op);
        void patchJump(std::size_t offset);
        void emitOpCode(OpCode op);
        void emitByte(std::uint8_t byte);
        void emitBytes(std::uint8_t a, std::uint8_t b);
        std::size_t currentChunkCodeOffset() const;

        // statement parsers
        void declaration();
        void classDeclaration();
        void method();
        void funDeclaration();
        void function(FunctionType type, const Token& name);
        void varDeclaration();
        void parseVariable(std::size_t& idx, std::string_view msg);
        void declareVariable(const Token& t);
        void defineVariable(std::size_t idx);
        void statement();
        void ifStatement();
        void whileStatement();
        void forStatement();
        void returnStatement();
        void block();
        void printStatement();
        void expressionStatement();

        // expression parsers
        static ParseRule getParseRule(TokenType t);
        void parsePrecedence(OpPrecedence p);
        void expression();
        void call(bool canAssign);
        unsigned argList();
        void variable(bool canAssign);
        void namedVariable(const Token& t, bool canAssign);
        void number(bool canAssign);
        void grouping(bool canAssign);
        void unary(bool canAssign);
        void binary(bool canAssign);
        void literal(bool canAssign);
        void string(bool canAssign);
        void and_(bool canAssign);
        void or_(bool canAssign);
        void dot(bool canAssign);
        void _this(bool canAssign);
        void super(bool canAssign);

    private:
        std::string source = "";
        Scanner scanner;
        struct Parser {
            Token previous;
            Token current;
            bool hadError = false;
            bool panicMode = false;
        } parser;
        struct Frame {
            Frame* parent = nullptr;
            FunctionType funType = FunctionType::SCRIPT;
            Function* function = nullptr;
            Vector<Local> locals;
            Vector<Upvalue> upvalues;
            std::uint16_t scopeDepth = 0;
        } frame;
        struct CompilerClass {
            CompilerClass* parent = nullptr;
            bool null = true;
            bool hasSuperclass = false;
        } enclosingClass;
        Vector<Object*> gcObjects;
        DiagnosticEngine* diagnostics = nullptr;
    };
} // namespace cpplox