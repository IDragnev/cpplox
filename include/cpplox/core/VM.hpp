#pragma once

#include "cpplox/core/Chunk.hpp"
#include "cpplox/core/ValueStack.hpp"

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

    private:
        const std::uint8_t* ip = nullptr;
        const Chunk* chunk = nullptr;
        ValueStack stack;
    };

    template <NumberBinaryOp Op>
    bool VM::numBinaryOp(const Op& op) {
        using value::isNumber;
        using value::asNumber;

        if (isNumber(stack.peek()) && isNumber(stack.peekN(1))) {
            const double b = asNumber(stack.pop());
            const double a = asNumber(stack.pop());
            stack.push(op(a, b));

            return true;
        }

        // report runtime error
        return false;
    }
} // namespace cpplox