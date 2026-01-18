#include "cpplox/vm/VM.hpp"
#include "cpplox/bytecode/Disassembler.hpp"
#include "cpplox/bytecode/OpCode.hpp"
#include "cpplox/bytecode/Bytecode.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/Function.hpp"
#include "cpplox/log/Log.hpp"

#include "cpplox/core/Format.hpp"

namespace cpplox {
    VM::~VM() {
        // ok for now, will be managed by the runtime GC
        const std::size_t cnt = gcObjects.getCount();
        for (std::size_t i = 0; i < cnt; ++i) {
            delete gcObjects[i];
        }
        gcObjects.clear();
    }

    InterpretResult VM::interpret(Function* func, Vector<Object*>&& objects) {
        if (func == nullptr) {
            return InterpretResult::COMPILE_ERROR;
        }
        if (func->chunk.code.isEmpty()) {
            return InterpretResult::OK;
        }

        addObjects(std::move(objects));
        stack.reserve(2056);
        frames.reserve(512);

        stack.push(Value(func));
        call(func, 0);

        auto r = run();

        frames.clear();
        stack.clear();

        return r;
    }

    void VM::addObjects(Vector<Object*>&& objects) {
        const std::size_t cnt = objects.getCount();
        for (std::size_t i = 0; i < cnt; ++i) {
            gcObjects.insertBack(objects[i]);
        }
        objects.clear();
    }

    InterpretResult VM::run() {
        CallFrame* frame = &frames.back();

        const auto readByte = [&frame] {
            return *(frame->ip++);
        };
        const auto readIdx16 = [&readByte] {
            auto a = readByte();
            auto b = readByte();
            return parseTwoByteInteger(a, b);
        };
        const auto readConstant = [&frame, &readByte] {
            return frame->function->chunk.constants[readByte()];
        };
        const auto readConstant16 = [&frame, &readIdx16] {
            return frame->function->chunk.constants[readIdx16()];
        };

#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
        Disassembler disassembler;
#endif

        for (;;) {
#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
            disassembler.disassembleInstruction(frame->function->chunk,
                                                frame->ip - frame->function->chunk.code.data());
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
                case OpCode::POP_N:
                case OpCode::POP_N_16: {
                    const std::size_t n =
                        opCode == OpCode::POP_N ? readByte() : readIdx16();
                    stack.popN(n);
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
                case OpCode::READ_LOCAL:
                case OpCode::READ_LOCAL_16: {
                    std::size_t idx =
                        opCode == OpCode::READ_LOCAL ? readByte() : readIdx16();
                    idx += frame->bp;
                    stack.push(stack.at(idx));
                } break;
                case OpCode::SET_LOCAL:
                case OpCode::SET_LOCAL_16: {
                    std::size_t idx =
                        opCode == OpCode::SET_LOCAL ? readByte() : readIdx16();
                    idx += frame->bp;
                    stack.at(idx) = stack.peek();
                } break;
                case OpCode::JMP_IF_FALSE: {
                    const std::size_t offset = readIdx16();
                    if (stack.peek().isFalsey()) {
                        frame->ip += offset;
                    }
                } break;
                case OpCode::JMP: {
                    const std::size_t offset = readIdx16();
                    frame->ip += offset;
                } break;
                case OpCode::LOOP: {
                    const std::size_t offset = readIdx16();
                    frame->ip -= offset;
                } break;
                case OpCode::CALL: {
                    const std::uint8_t argc = readByte();
                    const Value& fun = stack.peekN(argc);
                    if (callValue(fun, argc) == false) {
                        return InterpretResult::RUNTIME_ERROR;
                    }
                    frame = &frames.back();
                } break;
                case OpCode::RETURN: {
                    Value result = stack.pop();
                    const std::size_t poppedBP = frame->bp;

                    frames.removeBack();
                    if (frames.isEmpty()) {
                        return InterpretResult::OK;
                    }
                    frame = &frames.back();

                    const std::size_t popCnt = stack.size() - poppedBP;
                    stack.popN(popCnt);
                    stack.push(result);
                } break;
                default: {
                    return InterpretResult::RUNTIME_ERROR;
                } break;
            }

#undef BINARY_OP
        }
    }

    bool VM::callValue(const Value& v, std::uint8_t argc) {
        if (v.isObject()) {
            const Function* fun = v.asObject()->as<Function>();
            if (fun != nullptr) {
                return call(fun, argc);
            }
        }

        runtimeError("Can only call functions and classes.");
        return false;
    }

    bool VM::call(const Function* f, std::uint8_t argc) {
        if (f->arity != argc) {
            // todo: add expected and actual count
            runtimeError("Invalid argument count");
            return false;
        }

        frames.insertBack(CallFrame{
            .function = f,
            .ip = f->chunk.code.data(),
            .bp = stack.size() - argc - 1,
        });

#ifdef CPPLOX_DEBUG_TRACE_EXECUTION
        Disassembler disassembler;
        disassembler.disassembleChunk(f->chunk, f->name.c_str());
#endif

        return true;
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
        errorln("Runtime error: {}", msg);

        for (std::size_t i = frames.getCount(); i > 0; --i) {
            const CallFrame& frame = frames[i - 1];
            const Chunk& chunk = frame.function->chunk;

            std::size_t instruction = frame.ip - chunk.code.data() - 1;
            if (instruction <= chunk.lines.getCount()) {
                unsigned line = chunk.lines[instruction];
                errorln("[line {}] in {}",
                        line,
                        i != 1 ? frame.function->name : "script");
            }
        }
    }
}
