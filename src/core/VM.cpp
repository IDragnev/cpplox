#include "cpplox/core/VM.hpp"
#include "cpplox/debug/Disassembler.hpp"

namespace cpplox {
    InterpretResult VM::interpret(const Chunk& c) {
        stack.clear();
        chunk = &c;
        ip = chunk->code.data();

        return run();
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

            const auto instruction = readByte();
            switch (static_cast<OpCode>(instruction)) {
                case OpCode::ADD: {
                    binaryOp([](const Value& a, const Value& b) {
                        return a + b;
                    });
                } break;
                case OpCode::SUBTRACT: {
                    binaryOp([](const Value& a, const Value& b) {
                        return a - b;
                    });
                } break;
                case OpCode::DIVIDE: {
                    binaryOp([](const Value& a, const Value& b) {
                        return a / b;
                    });
                } break;
                case OpCode::MULTIPLY: {
                    binaryOp([](const Value& a, const Value& b) {
                        return a * b;
                    });
                } break;
                case OpCode::NEGATE: {
                    stack.peek() *= -1;
                } break;
                case OpCode::CONSTANT: {
                    stack.push(readConstant());
                } break;
                case OpCode::RETURN: {
                    printValue(stack.pop());
                    printf("\n");
                    return InterpretResult::OK;
                } break;
                default: {
                    return InterpretResult::OK;
                } break;
            }
        }
    }
}