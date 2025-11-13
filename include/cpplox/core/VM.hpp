#pragma once

#include "cpplox/core/Chunk.hpp"
#include "cpplox/core/ValueStack.hpp"

#include <stdio.h>

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

        /*template <typename... Args>
        void runtimeError(Args&&... args);*/
        void runtimeError(const char* msg);

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

        runtimeError("Operands must be numbers.");
        return false;
    }

    /*
    template <typename... Args>
    void VM::runtimeError(Args&&... args) {
        fprintf(stderr, "Runtime error: ");
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"
        fprintf(stderr, std::forward<Args>(args)...);
    #pragma GCC diagnostic pop
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-security"
        fprintf(stderr, std::forward<Args>(args)...);
    #pragma clang diagnostic pop
#elif defined(_MSC_VER)
        fprintf(stderr, std::forward<Args>(args)...);
#endif
        fputs("\n", stderr);

        std::size_t instruction = this->ip - this->chunk->code.data() - 1;
        if (instruction <= this->chunk->lines.size()) {
            unsigned line = this->chunk->lines[instruction];
            fprintf(stderr, "[line %u] in script\n", line);
        }
    }
    */
} // namespace cpplox