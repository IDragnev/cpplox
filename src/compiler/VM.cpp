#include "cpplox/compiler/VM.hpp"
#include "cpplox/compiler/Disassembler.hpp"
#include "cpplox/log/Log.hpp"
#include <stdio.h>

#include "cpplox/core/Format.hpp"

namespace cpplox {
    InterpretResult VM::interpret(const Chunk& c) {
        if (c.code.isEmpty()) {
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
        const auto readConstant16 = [this, &readByte] {
            auto a = readByte();
            auto b = readByte();
            const auto i = parseConstant16Index(a, b);
            return chunk->constants[i];
        };

#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
        Disassembler disassembler;
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

            const auto opCode = static_cast<OpCode>(readByte());
            switch (opCode) {
                case OpCode::ADD: {
                    if (stack.peek().isString() && stack.peekN(1).isString()) {
                        Value b = stack.pop();
                        Value& a = stack.peek();
                        a.asString() += b.asString();
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
                    stack.push(readConstant16());
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
                case OpCode::PRINT: {
                    Value v = stack.pop();
                    println("{}", v);
                } break;
                case OpCode::POP: {
                    stack.pop();
                } break;
                case OpCode::DEFINE_GLOBAL:
                case OpCode::DEFINE_GLOBAL_16: {
                    Value name = opCode == OpCode::DEFINE_GLOBAL
                                     ? readConstant()
                                     : readConstant16();
                    if (name.isString()) {
                        globals.insert(name.asString(), stack.peek());
                        stack.pop();
                    } else {
                        runtimeError("Internal compiler error.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                case OpCode::READ_GLOBAL:
                case OpCode::READ_GLOBAL_16: {
                    Value name = opCode == OpCode::READ_GLOBAL
                                     ? readConstant()
                                     : readConstant16();
                    if (name.isString()) {
                        Value value;
                        bool exists = globals.find(name.asString(), value);
                        if (exists) {
                            stack.push(value);
                        } else {
                            String error = "Undefined variable '";
                            error += name.asString();
                            error += "'.";
                            runtimeError(error.c_str());

                            return InterpretResult::RUNTIME_ERROR;
                        }
                    } else {
                        runtimeError("Internal compiler error.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                case OpCode::SET_GLOBAL:
                case OpCode::SET_GLOBAL_16: {
                    Value name = opCode == OpCode::SET_GLOBAL
                                     ? readConstant()
                                     : readConstant16();
                    if (name.isString()) {
                        bool exists = globals.contains(name.asString());
                        if (exists) {
                            globals.insert(name.asString(), stack.peek());
                        } else {
                            String error = "Undefined variable '";
                            error += name.asString();
                            error += "'.";
                            runtimeError(error.c_str());

                            return InterpretResult::RUNTIME_ERROR;
                        }
                    } else {
                        runtimeError("Internal compiler error.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                } break;
                default: {
                    return InterpretResult::OK;
                } break;
            }

#undef BINARY_OP
        }
    }

    template <NumberBinaryOp Op>
    bool VM::numBinaryOp(const Op& op) {
        if (stack.peek().isNumber() && stack.peekN(1).isNumber()) {
            const double b = stack.pop().asNumber();
            const double a = stack.pop().asNumber();
            stack.push(op(a, b));

            return true;
        }

        runtimeError("Operands must be numbers.");
        return false;
    }

    void VM::runtimeError(const char* msg) {
        fprintf(stderr, "Runtime error: %s\n", msg);

        std::size_t instruction = this->ip - this->chunk->code.data() - 1;
        if (instruction <= this->chunk->lines.getCount()) {
            unsigned line = this->chunk->lines[instruction];
            fprintf(stderr, "[line %u] in script\n", line);
        }
    }
}
