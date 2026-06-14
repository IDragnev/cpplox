#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/bytecode/OpCode.hpp"
#include "cpplox/bytecode/Bytecode.hpp"
#include "cpplox/bytecode/Chunk.hpp"
#include "cpplox/runtime/Function.hpp"
#include "cpplox/runtime/GC.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"
#include "cpplox/core/Algorithm.hpp"

#include <limits>

namespace cpplox {
    static const std::size_t MAX_LOCALS =
        std::numeric_limits<std::uint16_t>::max() + 1;
    static const std::size_t MAX_UPVALUES = 255;
    static const std::size_t MAX_FUN_PARAMS = 255;

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
            rules[as_index(TokenType::LEFT_PAREN)]    = ParseRule{ .prefix = &Compiler::grouping,.infix = &Compiler::call,   .infixPrec = OpPrecedence::CALL, };
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
            rules[as_index(TokenType::DOT)]           = ParseRule{                               .infix = &Compiler::dot,    .infixPrec = OpPrecedence::CALL, };
            rules[as_index(TokenType::THIS)]          = ParseRule{ .prefix = &Compiler::_this, };
            rules[as_index(TokenType::BREAK)]         = ParseRule{ .prefix = &Compiler::_break, };
            rules[as_index(TokenType::SUPER)]         = ParseRule{ .prefix = &Compiler::super, };
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

    Compiler::Compiler()
        : source("")
        , scanner(source, nullptr)
    {
    }

    CompileResult Compiler::replExpression(std::string src,
                                           DiagnosticEngine* engine) {
        bool initOk = init(std::move(src), engine);
        if (initOk) {
            advance();
            expression();
            emitOpCode(OpCode::PRINT);
            emitReturn();
        }

        bool error = initOk == false ||
                     peek(TokenType::EOF_TOKEN) == false ||
                     parser.hadError;
        CompileResult result = prepareResult(error);

        cleanUp();

        return result;
    }

    CompileResult Compiler::compile(std::string src, DiagnosticEngine* diag) {
        bool initOk = init(std::move(src), diag);
        if (initOk) {
            advance();
            do {
                declaration();

                if (parser.panicMode) {
                    synchronize();
                }
            } while (peek(TokenType::EOF_TOKEN) == false);
            emitReturn();
        }

        bool error = initOk == false || parser.hadError;
        CompileResult result = prepareResult(error);

        cleanUp();

        return result;
    }

    bool Compiler::init(std::string&& compileSource, DiagnosticEngine* engine) {
        source = std::move(compileSource);
        diagnostics = engine;
        scanner = Scanner(source, engine);
        gcObjects.reserve(256);

        Function* scriptFun = makeObject<Function>("<script>");
        if (scriptFun != nullptr) {
            initFrame(frame, scriptFun, FunctionType::SCRIPT, nullptr);
            return true;
        }

        return false;
    }

    void Compiler::initFrame(Frame& fr, Function* f, FunctionType t, Frame* parent) {
        fr.funType = t;
        fr.function = f;
        fr.scopeDepth = 0;
        fr.parent = parent;
        fr.locals.reserve(MAX_LOCALS);
        fr.upvalues.reserve(MAX_UPVALUES);

        const bool method = t == FunctionType::METHOD || t == FunctionType::INITIALIZER;
        // reserved for the function being compiled
        fr.locals.insertBack(Local{
            .name = Token{.lexeme = method ? "this" : ""},
            .depth = 0,
            .initialized = true,
            .captured = false,
        });
    }

    CompileResult Compiler::prepareResult(bool hadError) {
        CompileResult result;
        result.error = hadError;
        if (result.error == false) {
            result.gcObjects = std::move(gcObjects);
            result.function = frame.function;
        }

        return result;
    }

    void Compiler::cleanUp() {
        source = "";
        diagnostics = nullptr;
        scanner = Scanner(source, nullptr);
        parser = Parser{};
        frame = Frame{};

        const std::size_t size = gcObjects.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            gc::freeObject(gcObjects[i]);
        }
        gcObjects.clear();
    }

    template <typename T, typename... Args>
    T* Compiler::makeObject(Args&&... args) {
        T* obj = gc::makeObject<T>(std::forward<Args>(args)...);
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

        while (peek(TokenType::EOF_TOKEN) == false) {
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

    bool Compiler::inLocalScope() const {
        return frame.scopeDepth > 0;
    }

    void Compiler::beginScope() {
        ++frame.scopeDepth;
    }

    void Compiler::endScope() {
        if (frame.scopeDepth > 0) {
            --frame.scopeDepth;
        }

        std::uint16_t popCount = 0;
        while (frame.locals.isEmpty() == false &&
               frame.locals.back().depth > frame.scopeDepth)
        {
            bool captured = frame.locals.back().captured;
            if (captured == false) {
                ++popCount;
            } else {
                if (popCount > 0) {
                    emitIntegerInstruction(OpCode::POP_N,
                                           OpCode::POP_N_16,
                                           popCount);
                    popCount = 0;
                }
                emitOpCode(OpCode::CLOSE_UPVALUE);
            }

            frame.locals.removeBack();
        }

        if (popCount > 0) {
            emitIntegerInstruction(OpCode::POP_N, OpCode::POP_N_16, popCount);
        }
    }

    void Compiler::addLocal(const Token& name) {
        if (frame.locals.getCount() == MAX_LOCALS) {
            compileError(
                name,
                "Can't have more than {} local variables in a function",
                MAX_LOCALS);
            return;
        }

        frame.locals.insertBack(Local{
            .name = name,
            .depth = frame.scopeDepth,
            .initialized = false,
            .captured = false,
        });
    }

    void Compiler::addUpvalue(Frame& fr,
                              const Token& name,
                              Upvalue upvalue,
                              std::size_t& upvalueIdx) {
        const auto size = fr.upvalues.getCount();
        for (std::size_t i = 0; i < size; ++i) {
            if (fr.upvalues[i] == upvalue) {
                upvalueIdx = i;
                return;
            }
        }

        if (fr.upvalues.getCount() == MAX_UPVALUES) {
            compileError(name,
                         "Can't have more than {} captures in a closure",
                         MAX_UPVALUES);
            return;
        }

        upvalueIdx = fr.upvalues.getCount();

        fr.upvalues.insertBack(upvalue);
        fr.function->upvaluesCount++;
    }

    bool Compiler::resolveLocal(Frame& fr, const Token& name, std::size_t& idx) {
        for (std::size_t i = fr.locals.getCount(); i > 0; --i) {
            auto localIdx = i - 1;
            const Local& local = fr.locals[localIdx];
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

    bool Compiler::resolveUpvalue(Frame& fr,
                                  const Token& name,
                                  std::size_t& idx) {
        if (fr.parent == nullptr) {
            return false;
        }

        if (std::size_t local = 0;
            resolveLocal(*fr.parent, name, local)) {
            addUpvalue(fr, name, Upvalue{.index = local, .isLocal = true}, idx);
            fr.parent->locals[local].captured = true;
            return true;
        }

        if (std::size_t upvalue = 0;
            resolveUpvalue(*fr.parent, name, upvalue)) {
            addUpvalue(fr,
                       name,
                       Upvalue{.index = upvalue, .isLocal = false},
                       idx);
            return true;
        }

        return false;
    }

    void Compiler::advance() {
        if (parser.current.type == TokenType::EOF_TOKEN) {
            return;
        }

        parser.previous = parser.current;

        if (scanner.isDone()) {
            ScanResult r = scanner.scanToken();
            parser.current = r.token;
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
        if (match(TokenType::CLASS)) {
            classDeclaration();
        } else if (match(TokenType::FUN)) {
            funDeclaration();
        } else if (match(TokenType::VAR)) {
            varDeclaration();
        } else {
            statement();
        }
    }

    void Compiler::classDeclaration() {
        consumeTokenErr(TokenType::IDENTIFIER, "Expected class name");
        const Token className = parser.previous;

        std::size_t idx = 0;
        makeConstant(Value(String(className.lexeme)), true, idx);
        declareVariable(className);
        emitIntegerInstruction(OpCode::MAKE_CLASS, OpCode::MAKE_CLASS_16, idx);
        defineVariable(idx);

        auto oldClass = enclosingClass;
        enclosingClass.null = false;
        enclosingClass.parent = &oldClass;

        if (match(TokenType::LESS)) {
            consumeTokenErr(TokenType::IDENTIFIER, "Expected superclass name");
            variable(false);
            if (parser.previous.lexeme == className.lexeme) {
                compileError(parser.previous, "A class can't inherit from itself");
            }

            beginScope();
            addLocal(Token{
                .lexeme = std::string_view("super"),
            });
            defineVariable(0);

            namedVariable(className, false);
            emitOpCode(OpCode::INHERIT);
            enclosingClass.hasSuperclass = true;
        }

        namedVariable(className, false);
        consumeTokenErr(TokenType::LEFT_BRACE, "Expected '{{' after class name");
        while (peek(TokenType::RIGHT_BRACE) == false &&
               peek(TokenType::EOF_TOKEN) == false)
        {
            method();
        }
        consumeTokenErr(TokenType::RIGHT_BRACE, "Expected '}}' after class body");
        emitOpCode(OpCode::POP);

        if (enclosingClass.hasSuperclass) {
            endScope();
        }

        enclosingClass = oldClass;
    }

    void Compiler::method() {
        consumeTokenErr(TokenType::IDENTIFIER, "Expected method name");

        std::size_t idx = 0;
        makeConstant(Value(String(parser.previous.lexeme)),
                         true,
                         idx);

        const auto type = parser.previous.lexeme == "init"
                              ? FunctionType::INITIALIZER
                              : FunctionType::METHOD;
        function(type, parser.previous);

        emitIntegerInstruction(OpCode::METHOD, OpCode::METHOD_16, idx);
    }

    void Compiler::funDeclaration() {
        std::size_t idx = 0;
        parseVariable(idx, "Expected function name");

        if (inLocalScope()) {
            // functions can refer to themselves in their bodies
            if (frame.locals.getCount() > 0) {
                frame.locals.back().initialized = true;
            }
        }

        function(FunctionType::FUNCTION, parser.previous);
        defineVariable(idx);
    }

    void Compiler::function(FunctionType type, const Token& name) {
        auto* fun = makeObject<Function>(name.lexeme);
        if (fun == nullptr) {
            return;
        }

        Loop oldLoop = std::move(loop);
        loop.null = true;

        Frame oldFrame = std::move(frame);
        frame = Frame{};
        initFrame(frame, fun, type, &oldFrame);

        beginScope();

        consumeTokenErr(TokenType::LEFT_PAREN,
                        "Expected '(' after function name");
        if (peek(TokenType::RIGHT_PAREN) == false) {
            do {
                ++(frame.function->arity);
                if (frame.function->arity > MAX_FUN_PARAMS) {
                    compileError(parser.current,
                                 "Can't have more than {} parameters",
                                 MAX_FUN_PARAMS);
                    break;
                }

                std::size_t idx = 0;
                parseVariable(idx, "Expected parameter name");
                defineVariable(idx);
            } while (match(TokenType::COMMA));
        }
        consumeTokenErr(TokenType::RIGHT_PAREN,
                        "Expected ')' after parameters");
        consumeTokenErr(TokenType::LEFT_BRACE,
                        "Expected '{{' before function body");
        block();
        emitReturn();

        // no need for endScope because we
        // return to compiling the parent function
        Frame closureFrame = std::move(frame);
        frame = std::move(oldFrame);
        emitClosure(fun, closureFrame);

        loop = std::move(oldLoop);
    }

    void Compiler::varDeclaration() {
        std::size_t idx = 0;
        parseVariable(idx, "Expected variable name");

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

    void Compiler::parseVariable(std::size_t& idx, std::string_view msg) {
        consumeTokenErr(TokenType::IDENTIFIER, msg);

        if (inLocalScope()) {
            declareVariable(parser.previous);
        } else {
            makeConstant(Value(String(parser.previous.lexeme)),
                         true,
                         idx);
        }
    }

    void Compiler::declareVariable(const Token& name) {
        if (inLocalScope() == false) {
            return;
        }

        for (std::size_t i = frame.locals.getCount(); i > 0; --i) {
            const Local& loc = frame.locals[i - 1];

            if (loc.initialized && loc.depth < frame.scopeDepth) {
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
        if (inLocalScope()) {
            if (frame.locals.getCount() > 0) {
                frame.locals.back().initialized = true;
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
        } else if (match(TokenType::RETURN)) {
            returnStatement();
        } else if (match(TokenType::LEFT_BRACE)) {
            beginScope();
            block();
            endScope();
        } else {
            expressionStatement();
        }
    }
    
    void Compiler::block() {
        while (peek(TokenType::EOF_TOKEN) == false &&
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
        auto oldLoop = std::move(loop);
        loop.null = false;

        const std::size_t loopStart = currentChunkCodeOffset();

        consumeTokenErr(TokenType::LEFT_PAREN, "Expected '(' after while");
        expression();
        consumeTokenErr(TokenType::RIGHT_PAREN, "Expected ')' after condition");

        std::size_t exitJmp = emitJump(OpCode::JMP_IF_FALSE);
        emitOpCode(OpCode::POP);
        statement();
        emitLoop(loopStart);

        patchJump(exitJmp);
        forEach(loop.breaksToPatch,
                [this](std::size_t jmp) { patchJump(jmp); });
        emitOpCode(OpCode::POP);

        loop = std::move(oldLoop);
    }

    void Compiler::forStatement() {
        auto oldLoop = std::move(loop);
        loop.null = false;

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

        forEach(loop.breaksToPatch,
                [this](std::size_t jmp) { patchJump(jmp); });
        if (hasCondition) {
            patchJump(exitJmp);
            emitOpCode(OpCode::POP); // condition
        }

        endScope();

        loop = std::move(oldLoop);
    }

    void Compiler::returnStatement() {
        if (frame.funType == FunctionType::SCRIPT) {
            compileError(parser.previous, "Can't return from top-level code");
            return;
        }

        if (match(TokenType::SEMICOLON)) {
            emitReturn();
        } else {
            if (frame.funType == FunctionType::INITIALIZER) {
                compileError(parser.previous,
                             "Can't return a value from an initializer");
            }

            expression();
            consumeTokenErr(TokenType::SEMICOLON, "Expected ';' after return");
            emitOpCode(OpCode::RETURN);
        }
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

    void Compiler::call(bool) {
        unsigned count = argList();
        emitOpCode(OpCode::CALL);
        emitByte(static_cast<std::uint8_t>(count));
    }

    unsigned Compiler::argList() {
        unsigned count = 0;

        if (peek(TokenType::RIGHT_PAREN) == false) {
            do {
                ++count;
                if (count > MAX_FUN_PARAMS) {
                    compileError(parser.current, "Can't have more than {} arguments", MAX_FUN_PARAMS);
                    break;
                }
                expression();
            } while (match(TokenType::COMMA));
        }
        consumeTokenErr(TokenType::RIGHT_PAREN,
                        "Expected ')' after function arguments");

        return count;
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
        bool upvalue = false;
        bool local = resolveLocal(frame, t, idx);
        if (local == false) {
            upvalue = resolveUpvalue(frame, t, idx);
            if (upvalue == false) {
                makeConstant(Value(String(t.lexeme)), true, idx);
            }
        }

        if (canAssign && match(TokenType::EQUAL)) {
            expression();
            if (local) {
                emitIntegerInstruction(OpCode::SET_LOCAL,
                                       OpCode::SET_LOCAL_16,
                                       idx);
            } else if (upvalue) {
                emitOpCode(OpCode::SET_UPVALUE);
                emitByte(static_cast<std::uint8_t>(idx));
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
            } else if (upvalue) {
                emitOpCode(OpCode::READ_UPVALUE);
                emitByte(static_cast<std::uint8_t>(idx));
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

    void Compiler::dot(bool canAssign) {
        consumeTokenErr(TokenType::IDENTIFIER,
                        "Expected property name after '.'");

        std::size_t idx = 0;
        makeConstant(Value(String(parser.previous.lexeme)), true, idx);

        if (canAssign && match(TokenType::EQUAL)) {
            expression();
            emitIntegerInstruction(OpCode::SET_PROPERTY,
                                   OpCode::SET_PROPERTY_16,
                                   idx);
        } else if (match(TokenType::LEFT_PAREN)) {
            const auto count = argList();
            emitIntegerInstruction(OpCode::INVOKE, OpCode::INVOKE_16, idx);
            emitByte(static_cast<std::uint8_t>(count));
        } else {
            emitIntegerInstruction(OpCode::GET_PROPERTY,
                                   OpCode::GET_PROPERTY_16,
                                   idx);
        }
    }

    void Compiler::_this(bool) {
        if (enclosingClass.null) {
            compileError(parser.previous, "Can't use 'this' outside of class");
        }

        variable(false);
    }

    void Compiler::_break(bool) {
        if (loop.null) {
            compileError(parser.previous, "Can't use 'break' outside of loop");
        }

        std::size_t jmp = emitJump(OpCode::JMP);
        loop.breaksToPatch.insertBack(jmp);
    }

    void Compiler::super(bool) {
        if (enclosingClass.null) {
            compileError(parser.previous,
                         "Can't use 'super' outside of a class");
        } else if (enclosingClass.hasSuperclass == false) {
            compileError(parser.previous,
                         "Can't use 'super' in a class with no superclass");
        }

        consumeTokenErr(TokenType::DOT, "Expected '.' after 'super'");
        consumeTokenErr(TokenType::IDENTIFIER, "Expected superclass method name");

        std::size_t idx = 0;
        makeConstant(Value(String(parser.previous.lexeme)), true, idx);

        namedVariable(Token{.lexeme = "this"}, false);
        if (match(TokenType::LEFT_PAREN)) {
            const unsigned argc = argList();
            namedVariable(Token{.lexeme = "super"}, false);
            emitIntegerInstruction(OpCode::SUPER_INVOKE,
                                   OpCode::SUPER_INVOKE_16,
                                   idx);
            emitByte(static_cast<std::uint8_t>(argc));
        } else {
            namedVariable(Token{.lexeme = "super"}, false);
            emitIntegerInstruction(OpCode::GET_SUPER,
                                   OpCode::GET_SUPER_16,
                                   idx);
        }
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
            const Vector<Value>& constants = frame.function->chunk.constants;
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
            idx = addConstant(frame.function->chunk, std::move(value));
        }

        bool success = true;
        if (fitsTwoBytes(idx) == false) {
            compileError(parser.previous, "Constants limits reached");
            success = false;
        }

        return success;
    }

    void Compiler::emitClosure(Function* fun, const Frame& closureFrame) {
        std::size_t idx = 0;
        bool ok = makeConstant(Value(fun), false, idx);
        if (ok) {
            emitIntegerInstruction(OpCode::MAKE_CLOSURE,
                                   OpCode::MAKE_CLOSURE_16,
                                   idx);
        }

        const auto count = closureFrame.upvalues.getCount();
        emitByte(static_cast<std::uint8_t>(count));
        for (std::size_t i = 0; i < count; ++i) {
            const Upvalue& u = closureFrame.upvalues[i];
            emitByte(u.isLocal ? 1 : 0);
            if (u.isLocal) {
                std::uint8_t a = 0;
                std::uint8_t b = 0;
                serializeTwoByteInteger(u.index, a, b);
                emitBytes(a, b);
            } else {
                emitByte(static_cast<std::uint8_t>(u.index));
            }
        }
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

    void Compiler::emitReturn() {
        if (frame.funType != FunctionType::INITIALIZER) {
            emitOpCode(OpCode::NIL);
        } else {
            emitIntegerInstruction(OpCode::READ_LOCAL,
                                   OpCode::READ_LOCAL_16,
                                   0);
        }
        emitOpCode(OpCode::RETURN);
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

        return frame.function->chunk.code.getCount() - 2;
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

        frame.function->chunk.code[offset] = a;
        frame.function->chunk.code[offset + 1] = b;
    }

    void Compiler::emitOpCode(OpCode op) {
        emitByte(static_cast<std::uint8_t>(op));
    }

    void Compiler::emitByte(std::uint8_t byte) {
        addCode(frame.function->chunk, byte, parser.previous.line);
    }

    void Compiler::emitBytes(std::uint8_t a, std::uint8_t b) {
        emitByte(a);
        emitByte(b);
    }

    std::size_t Compiler::currentChunkCodeOffset() const {
        return frame.function->chunk.code.getCount();
    }
} // namespace cpplox