#pragma once

#include "cpplox/core/ValueStack.hpp"
#include "cpplox/core/ValueMap.hpp"
#include "cpplox/compiler/Chunk.hpp"

namespace cpplox {
    enum class InterpretResult {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    template <typename Op>
    concept NumberBinaryOp =
        requires(Op op, double a, double b, Value c) { c = op(a, b); };

    class VM {
    public:
        VM() = default;
        ~VM() = default;

        VM(const VM&) = delete;
        VM& operator=(const VM&) = delete;

        InterpretResult interpret(const Chunk& chunk);

    private:
        InterpretResult run();

        template <NumberBinaryOp Op>
        bool numBinaryOp(const Op& op);

        void runtimeError(const char* msg);

    private:
        const std::uint8_t* ip = nullptr;
        const Chunk* chunk = nullptr;
        ValueStack stack;
        ValueMap globals;
    };

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
} // namespace cpplox