#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/bytecode/OpCode.hpp"
#include "cpplox/bytecode/Bytecode.hpp"
#include "cpplox/bytecode/Chunk.hpp"
#include "cpplox/runtime/Function.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"
#include <limits>

namespace cpplox {
    static const std::size_t MAX_LOCALS =
        std::numeric_limits<std::uint16_t>::max() + 1;

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

    using ParseFn = void (Compiler::*)(bool);

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
            rules[as_index(TokenType::AND)]           = ParseRule{                               .infix = &Compiler::and_,   .infixPrec = OpPrecedence::AND, };
            rules[as_index(TokenType::OR)]            = ParseRule{                               .infix = &Compiler::or_,    .infixPrec = OpPrecedence::OR, };
            rules[as_index(TokenType::NUMBER)]        = ParseRule{ .prefix = &Compiler::number, };
            rules[as_index(TokenType::TRUE)]          = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::FALSE)]         = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::NIL)]           = ParseRule{ .prefix = &Compiler::literal, };
            rules[as_index(TokenType::BANG)]          = ParseRule{ .prefix = &Compiler::unary, };
            rules[as_index(TokenType::STRING)]        = ParseRule{ .prefix = &Compiler::string, };
            rules[as_index(TokenType::IDENTIFIER)]    = ParseRule{ .prefix = &Compiler::variable, };
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

    Compiler::Compiler(DiagnosticEngine* e)
        : source("")
        , scanner(source, e)
        , diagnostics(e)
    {
    }

    bool Compiler::compile(std::string src, Function* &f, Vector<Object*>& objects) {
        bool initOk = init(std::move(src));
        if (initOk) {
            advance();
            while (scanner.isDone() == false) {
                declaration();

                if (parser.panicMode) {
                    synchronize();
                }
            }
            emitOpCode(OpCode::RETURN);
        }

        bool hadError = initOk == false || parser.hadError;
        if (hadError == false) {
            objects = std::move(gcObjects);
            f = function;
        }

        cleanUp();

        return hadError;
    }

    bool Compiler::init(std::string&& compileSource) {
        source = std::move(compileSource);
        scanner = Scanner(source, diagnostics);
        locals.reserve(MAX_LOCALS);
        gcObjects.reserve(256);
        funType = FunctionType::SCRIPT;
        function = makeObject<Function>("<script>");

        // reserved for the function being compiled
        locals.insertBack(Local{
            .name = Token{},
            .depth = 0,
            .initialized = true,
        });

        return function != nullptr;
    }

    void Compiler::cleanUp() {
        source = "";
        scanner = Scanner(source, diagnostics);
        parser = Parser{};
        function = nullptr;
        locals.clear();
        scopeDepth = 0;

        const std::size_t size = gcObjects.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            delete gcObjects[i];
        }
        gcObjects.clear();
    }

    template <typename T, typename... Args>
    T* Compiler::makeObject(Args&&... args) {
        T* obj = new(std::nothrow) T(std::forward<Args>(args)...);
        if (obj != nullptr) {
            gcObjects.insertBack(obj);
        }
        else {
            compileError(parser.previous, "Out of memory");
        }

        return obj;
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

    void Compiler::beginScope() {
        ++scopeDepth;
    }

    void Compiler::endScope() {
        if (scopeDepth > 0) {
            --scopeDepth;
        }

        std::uint16_t popCount = 0;
        while (locals.isEmpty() == false && locals.back().depth > scopeDepth) {
            locals.removeBack();
            ++popCount;
        }

        emitIntegerInstruction(OpCode::POP_N,
                               OpCode::POP_N_16,
                               popCount);
    }

    void Compiler::addLocal(const Token& name) {
        if (locals.getCount() == MAX_LOCALS) {
            compileError(name, "Too many local variables in a function");
            return;
        }

        locals.insertBack(Local{
            .name = name,
            .depth = scopeDepth,
            .initialized = false,
        });
    }

    bool Compiler::resolveLocal(const Token& name, std::size_t& idx) {
        for (std::size_t i = locals.getCount(); i > 0; --i) {
            auto localIdx = i - 1;
            const Local& local = locals[localIdx];
            if (local.name.lexeme == name.lexeme) {
                if (local.initialized == false) {
                    compileError(
                        name,
                        "Can't read a local variable in its initializer");
                }

                idx = localIdx;
                return true;
            }
        }

        return false;
    }

    void Compiler::advance() {
        parser.previous = parser.current;

        if (scanner.isDone()) {
            parser.current = Token{};
            return;
        }

        while (scanner.isDone() == false) {
            ScanResult r = scanner.scanToken();
            parser.current = r.token;

            if (r.error == false) {
                break;
            }
            else {
                processError();
            }
        }
    }

    void Compiler::declaration() {
        if (match(TokenType::VAR)) {
            varDeclaration();
        } else {
            statement();
        }
    }

    void Compiler::varDeclaration() {
        std::size_t idx = 0;
        parseVariable(idx);

        if (match(TokenType::EQUAL)) {
            expression();
        }
        else {
            emitOpCode(OpCode::NIL);
        }
        consumeTokenErr(TokenType::SEMICOLON,
                        "Expected ';' after variable declaration");

        defineVariable(idx);
    }

    void Compiler::parseVariable(std::size_t& idx) {
        consumeTokenErr(TokenType::IDENTIFIER, "Expected variable name");

        const bool local = scopeDepth > 0;
        if (local) {
            declareVariable(parser.previous);
        } else {
            makeConstant(Value(String(parser.previous.lexeme)),
                         true,
                         idx);
        }
    }

    void Compiler::declareVariable(const Token& name) {
        if (bool global = scopeDepth == 0; global) {
            return;
        }

        for (std::size_t i = locals.getCount(); i > 0; --i) {
            const Local& loc = locals[i - 1];

            if (loc.initialized && loc.depth < scopeDepth) {
                break;
            }

            if (loc.name.lexeme == name.lexeme) {
                compileError(
                    name,
                    "Variable with name '{}' already exists in this scope",
                    name.lexeme);
            }
        }

        addLocal(name);
    }

    void Compiler::defineVariable(std::size_t idx) {
        if (bool localScope = scopeDepth > 0; localScope) {
            if (locals.getCount() > 0) {
                locals.back().initialized = true;
            }
        } else {
            emitIntegerInstruction(OpCode::DEFINE_GLOBAL,
                                   OpCode::DEFINE_GLOBAL_16,
                                   idx);
        }
    }

    void Compiler::statement() {
        if (match(TokenType::PRINT)) {
            printStatement();
        } else if (match(TokenType::IF)) {
            ifStatement();
        } else if (match(TokenType::WHILE)) {
            whileStatement();
        } else if (match(TokenType::FOR)) {
            forStatement();
        } else if (match(TokenType::LEFT_BRACE)) {
            beginScope();
            block();
            endScope();
        } else {
            expressionStatement();
        }
    }
    
    void Compiler::block() {
        while (scanner.isDone() == false &&
               peek(TokenType::RIGHT_BRACE) == false)
        {
            declaration();
        }

        consumeTokenErr(TokenType::RIGHT_BRACE, "Expected '}}' after block");
    }

    void Compiler::ifStatement() {
        consumeTokenErr(TokenType::LEFT_PAREN, "Expected '(' after if");
        expression();
        consumeTokenErr(TokenType::RIGHT_PAREN, "Expected ')' after condition");

        std::size_t thenJmp = emitJump(OpCode::JMP_IF_FALSE);
        emitOpCode(OpCode::POP);
        statement();
        std::size_t elseJmp = emitJump(OpCode::JMP);
        patchJump(thenJmp);

        emitOpCode(OpCode::POP);
        if (match(TokenType::ELSE)) {
            statement();
        }
        patchJump(elseJmp);
    }

    void Compiler::whileStatement() {
        const std::size_t loopStart = currentChunkCodeOffset();

        consumeTokenErr(TokenType::LEFT_PAREN, "Expected '(' after while");
        expression();
        consumeTokenErr(TokenType::RIGHT_PAREN, "Expected ')' after condition");

        std::size_t exitJmp = emitJump(OpCode::JMP_IF_FALSE);
        emitOpCode(OpCode::POP);
        statement();
        emitLoop(loopStart);

        patchJump(exitJmp);
        emitOpCode(OpCode::POP);
    }

    void Compiler::forStatement() {
        beginScope();

        consumeTokenErr(TokenType::LEFT_PAREN, "Expected '(' after for");
        if (match(TokenType::SEMICOLON)) {
            // no initializer
        } else if (match(TokenType::VAR)) {
            varDeclaration();
        } else {
            expressionStatement();
        }

        bool hasCondition = false;
        std::size_t exitJmp = 0;
        std::size_t loopStart = currentChunkCodeOffset();
        if (match(TokenType::SEMICOLON) == false) {
            hasCondition = true;
            expression();
            consumeTokenErr(TokenType::SEMICOLON,
                            "Expected ';' after loop condition");
            exitJmp = emitJump(OpCode::JMP_IF_FALSE);
            emitOpCode(OpCode::POP); // condition
        }

        if (match(TokenType::RIGHT_PAREN) == false) {
            const std::size_t bodyJmp = emitJump(OpCode::JMP);
            const std::size_t increment = currentChunkCodeOffset();
            expression();
            emitOpCode(OpCode::POP);
            consumeTokenErr(TokenType::RIGHT_PAREN, "Expected ')' after for clauses");

            emitLoop(loopStart);
            loopStart = increment;
            patchJump(bodyJmp);
        }

        statement();
        emitLoop(loopStart);

        if (hasCondition) {
            patchJump(exitJmp);
            emitOpCode(OpCode::POP); // condition
        }

        endScope();
    }

    void Compiler::printStatement() {
        expression();
        consumeTokenErr(TokenType::SEMICOLON, "Expected ';' after expression");
        emitOpCode(OpCode::PRINT);
    }

    void Compiler::expressionStatement() {
        expression();
        consumeTokenErr(TokenType::SEMICOLON, "Expected ';' after expression");
        emitOpCode(OpCode::POP);
    }

    void Compiler::expression() {
        parsePrecedence(OpPrecedence::ASSIGNMENT);
    }

    void Compiler::parsePrecedence(OpPrecedence precedence) {
        advance();

        const bool canAssign = precedence <= OpPrecedence::ASSIGNMENT;

        ParseRule rule = getParseRule(parser.previous.type);
        ParseFn prefixRule = rule.prefix;
        if (prefixRule == nullptr) {
            compileError(parser.previous, "Expected expression");
            return;
        }

        (this->*prefixRule)(canAssign);

        while (precedence <= getParseRule(parser.current.type).infixPrec) {
            advance();
            ParseRule r = getParseRule(parser.previous.type);
            ParseFn infixRule = r.infix;
            if (infixRule != nullptr) {
                (this->*infixRule)(canAssign);
            }
        }

        if (canAssign && match(TokenType::EQUAL)) {
            compileError(parser.previous, "Invalid assignment target");
        }
    }

    void Compiler::grouping(bool) {
        expression();
        consumeTokenErr(TokenType::RIGHT_PAREN, "Missing closing ')'");
    }

    void Compiler::binary(bool) {
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

    void Compiler::literal(bool) {
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

    void Compiler::unary(bool) {
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

    void Compiler::variable(bool canAssign) {
        namedVariable(parser.previous, canAssign);
    }

    void Compiler::namedVariable(const Token& t, bool canAssign) {
        std::size_t idx = 0;
        bool local = resolveLocal(t, idx);
        if (local == false) {
            makeConstant(Value(String(t.lexeme)), true, idx);
        }

        if (canAssign && match(TokenType::EQUAL)) {
            expression();
            if (local) {
                emitIntegerInstruction(OpCode::SET_LOCAL,
                                       OpCode::SET_LOCAL_16,
                                       idx);
            } else {
                emitIntegerInstruction(OpCode::SET_GLOBAL,
                                       OpCode::SET_GLOBAL_16,
                                       idx);
            }
        } else {
            if (local) {
                emitIntegerInstruction(OpCode::READ_LOCAL,
                                       OpCode::READ_LOCAL_16,
                                       idx);
            } else {
                emitIntegerInstruction(OpCode::READ_GLOBAL,
                                       OpCode::READ_GLOBAL_16,
                                       idx);
            }
        }
    }

    void Compiler::number(bool) {
        double v = std::strtod(parser.previous.lexeme.data(), nullptr);
        emitConstant(Value(v));
    }

    void Compiler::string(bool) {
        std::string_view v = parser.previous.lexeme;
        // trim quotes
        v.remove_prefix(1);
        v.remove_suffix(1);

        emitConstant(Value(v));
    }

    void Compiler::and_(bool) {
        std::size_t jmp = emitJump(OpCode::JMP_IF_FALSE);
        emitOpCode(OpCode::POP);
        parsePrecedence(OpPrecedence::AND);

        patchJump(jmp);
    }

    void Compiler::or_(bool) {
        std::size_t elseJmp = emitJump(OpCode::JMP_IF_FALSE);
        std::size_t endJmp = emitJump(OpCode::JMP);

        patchJump(elseJmp);
        emitOpCode(OpCode::POP);
        parsePrecedence(OpPrecedence::OR);

        patchJump(endJmp);
    }

    bool Compiler::match(TokenType type) {
        if (parser.current.type == type) {
            advance();
            return true;
        }

        return false;
    }

    bool Compiler::peek(TokenType type) const {
        return parser.current.type == type;
    }

    bool Compiler::consumeToken(TokenType tokenType) {
        if (parser.current.type == tokenType) {
            advance();
            return true;
        }

        return false;
    }

    template <typename... Args>
    void Compiler::consumeTokenErr(TokenType token,
                                   std::string_view fmt,
                                   Args&&... args) {
        bool ok = consumeToken(token);
        if (ok == false) {
            compileError(parser.current, fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void Compiler::compileError(const Token& t,
                                std::string_view fmt,
                                Args&&... args) {
        bool process = processError();
        if (process && diagnostics != nullptr) {
            diagnostics->report(t.line, fmt, std::forward<Args>(args)...);
        } 
    }

    bool Compiler::processError() {
        bool realError = false;
        if (parser.panicMode) {
            return realError;
        }

        realError = true;
        parser.panicMode = true;
        parser.hadError = true;

        return realError;
    }

    bool Compiler::makeConstant(Value value,
                                bool searchExisting,
                                std::size_t& idx) {
        bool found = false;
        if (searchExisting) {
            const Vector<Value>& constants = function->chunk.constants;
            const std::size_t size = constants.getCount();
            for (std::size_t i = 0; i < size; ++i) {
                if (constants[i] == value) {
                    idx = i;
                    found = true;
                    break;
                }
            }
        }
        if (found == false) {
            idx = addConstant(function->chunk, std::move(value));
        }

        bool success = true;
        if (fitsTwoBytes(idx) == false) {
            compileError(parser.previous, "Constants limits reached");
            success = false;
        }

        return success;
    }

    void Compiler::emitConstant(Value value) {
        std::size_t i = 0;
        bool ok = makeConstant(std::move(value), false, i);
        if (ok) {
            emitIntegerInstruction(OpCode::CONSTANT,
                                   OpCode::CONSTANT_16,
                                   i);
        }
    }

    void Compiler::emitIntegerInstruction(OpCode small,
                                          OpCode big,
                                          std::size_t operand) {
        if (fitsOneByte(operand)) {
            emitOpCode(small);
            emitByte(static_cast<std::uint8_t>(operand));
        } else if (fitsTwoBytes(operand)) {
            emitTwoByteIntegerInstruction(big, operand);
        }
    }

    void Compiler::emitTwoByteIntegerInstruction(OpCode op,
                                                 std::size_t operand) {
        std::uint8_t a = 0;
        std::uint8_t b = 0;
        serializeTwoByteInteger(operand, a, b);

        emitOpCode(op);
        emitBytes(a, b);
    }

    void Compiler::emitLoop(std::size_t loopStart) {
        const std::size_t current = currentChunkCodeOffset();
        if (current <= loopStart) {
            // guard against underflow
            return;
        }

        const std::size_t instructionSize = 3;
        std::size_t offset = current + instructionSize - loopStart;
        if (fitsTwoBytes(offset)) {
            emitTwoByteIntegerInstruction(OpCode::LOOP, offset);
        } else {
            compileError(parser.previous, "Loop body too large");
        }
    }

    std::size_t Compiler::emitJump(OpCode op) {
        emitOpCode(op);
        emitBytes(0xff, 0xff);

        return function->chunk.code.getCount() - 2;
    }

    void Compiler::patchJump(std::size_t offset) {
        const std::size_t JMP_ARGS_COUNT = 2;
        const std::size_t current = currentChunkCodeOffset();
        if (current <= offset || current - offset < JMP_ARGS_COUNT) {
            // guard against underflow
            return;
        }

        const std::size_t jmp = current - offset - JMP_ARGS_COUNT;
        if (fitsTwoBytes(jmp) == false) {
            compileError(parser.previous, "Too much code to jump over");
        }

        std::uint8_t a = 0;
        std::uint8_t b = 0;
        serializeTwoByteInteger(jmp, a, b);

        function->chunk.code[offset] = a;
        function->chunk.code[offset + 1] = b;
    }

    void Compiler::emitOpCode(OpCode op) {
        emitByte(static_cast<std::uint8_t>(op));
    }

    void Compiler::emitByte(std::uint8_t byte) {
        addCode(function->chunk, byte, parser.previous.line);
    }

    void Compiler::emitBytes(std::uint8_t a, std::uint8_t b) {
        emitByte(a);
        emitByte(b);
    }

    std::size_t Compiler::currentChunkCodeOffset() const {
        return function->chunk.code.getCount();
    }
} // namespace cpplox