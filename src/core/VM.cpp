#include "cpplox/core/VM.hpp"
#include "cpplox/debug/Disassembler.hpp"
#include <stdio.h>

namespace cpplox {
    InterpretResult VM::interpret(const Chunk& c) {
        if (c.code.empty()) {
            return InterpretResult::OK;
        }

        chunk = &c;
        ip = chunk->code.data();

        auto r = run();

        stack.clear();
        chunk = nullptr;
        ip = nullptr;

        return r;
    }

    InterpretResult VM::run() {
        const auto readByte = [this] {
            return *(ip++);
        };
        const auto readConstant = [this, &readByte] {
            return chunk->constants[readByte()];
        };

#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
        debug::Disassembler disassembler;
#endif

        for (;;) {
#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
            disassembler.disassembleInstruction(*chunk,
                                                ip - chunk->code.data());
#endif

#define BINARY_OP(op) \
            bool bOk = numBinaryOp([](double a, double b) { return Value(a op b); }); \
            if (bOk == false) { \
               return InterpretResult::RUNTIME_ERROR; \
            }

            const auto instruction = readByte();
            switch (static_cast<OpCode>(instruction)) {
                case OpCode::ADD: {
                    if (stack.peek().isString() && stack.peekN(1).isString()) {
                        Value b = stack.pop();
                        Value a = stack.pop();
                        stack.push(Value(a.asString() + b.asString()));
                    }
                    else if (stack.peek().isNumber() && stack.peekN(1).isNumber()) {
                        double b = stack.pop().asNumber();
                        double a = stack.pop().asNumber();
                        stack.push(Value(a + b));
                    } else {
                        runtimeError("Operands must be two numbers or two strings.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                case OpCode::SUBTRACT: {
                    BINARY_OP(-);
                } break;
                case OpCode::DIVIDE: {
                    BINARY_OP(/);
                } break;
                case OpCode::MULTIPLY: {
                    BINARY_OP(*);
                } break;
                case OpCode::LESS: {
                    BINARY_OP(<);
                } break;
                case OpCode::LESS_EQUAL: {
                    BINARY_OP(<=);
                } break;
                case OpCode::GREATER: {
                    BINARY_OP(>);
                } break;
                case OpCode::GREATER_EQUAL: {
                    BINARY_OP(>=);
                } break;
                case OpCode::EQUAL: {
                    Value b = stack.pop();
                    Value a = stack.pop();
                    stack.push(Value(a == b));
                } break;
                case OpCode::NOT_EQUAL: {
                    Value b = stack.pop();
                    Value a = stack.pop();
                    stack.push(Value(a != b));
                } break;
                case OpCode::NEGATE: {
                    if (stack.peek().isNumber()) {
                        double x = stack.pop().asNumber();
                        stack.push(Value(-x));
                    } else {
                        runtimeError("Operand must be a number.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                case OpCode::NOT: {
                    const bool v = stack.pop().isFalsey();
                    stack.push(Value(v));
                } break;
                case OpCode::CONSTANT: {
                    stack.push(readConstant());
                } break;
                case OpCode::CONSTANT_16: {
                    auto a = readByte();
                    auto b = readByte();
                    const auto i = parseConstant16Index(a, b);
                    stack.push(chunk->constants[i]);
                } break;
                case OpCode::TRUE: {
                    stack.push(Value(true));
                } break;
                case OpCode::FALSE: {
                    stack.push(Value(false));
                } break;
                case OpCode::NIL: {
                    stack.push(Value::nil());
                } break;
                case OpCode::RETURN: {
                    Value v = stack.pop();
                    v.print();
                    printf("\n");
                    return InterpretResult::OK;
                } break;
                default: {
                    return InterpretResult::OK;
                } break;
            }

#undef BINARY_OP
        }
    }

    void VM::runtimeError(const char* msg) {
        fprintf(stderr, "Runtime error: %s\n", msg);

        std::size_t instruction = this->ip - this->chunk->code.data() - 1;
        if (instruction <= this->chunk->lines.size()) {
            unsigned line = this->chunk->lines[instruction];
            fprintf(stderr, "[line %u] in script\n", line);
        }
    }
}
