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

#define BINARY_OP(op, asValue) \
            bool bOk = numBinaryOp([](double a, double b) { return (asValue)(a op b); }); \
            if (bOk == false) { \
               return InterpretResult::RUNTIME_ERROR; \
            }

            const auto instruction = readByte();
            switch (static_cast<OpCode>(instruction)) {
                case OpCode::ADD: {
                    BINARY_OP(+, value::number);
                } break;
                case OpCode::SUBTRACT: {
                    BINARY_OP(-, value::number);
                } break;
                case OpCode::DIVIDE: {
                    BINARY_OP(/, value::number);
                } break;
                case OpCode::MULTIPLY: {
                    BINARY_OP(*, value::number);
                } break;
                case OpCode::LESS: {
                    BINARY_OP(<, value::boolean);
                } break;
                case OpCode::LESS_EQUAL: {
                    BINARY_OP(<=, value::boolean);
                } break;
                case OpCode::GREATER: {
                    BINARY_OP(>, value::boolean);
                } break;
                case OpCode::GREATER_EQUAL: {
                    BINARY_OP(>=, value::boolean);
                } break;
                case OpCode::EQUAL: {
                    Value b = stack.pop();
                    Value a = stack.pop();
                    stack.push(value::boolean(a == b));
                } break;
                case OpCode::NOT_EQUAL: {
                    Value b = stack.pop();
                    Value a = stack.pop();
                    stack.push(value::boolean(a != b));
                } break;
                case OpCode::NEGATE: {
                    if (value::isNumber(stack.peek())) {
                        double x = value::asNumber(stack.pop());
                        stack.push(value::number(-x));
                    } else {
                        // report runtime error
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                case OpCode::NOT: {
                    const bool v = value::isFalsey(stack.pop());
                    stack.push(value::boolean(v));
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
                    stack.push(value::TRUE);
                } break;
                case OpCode::FALSE: {
                    stack.push(value::FALSE);
                } break;
                case OpCode::NIL: {
                    stack.push(value::NIL);
                } break;
                case OpCode::RETURN: {
                    value::print(stack.pop());
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
}