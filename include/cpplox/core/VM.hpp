#pragma once

#include "cpplox/core/Chunk.hpp"
#include "cpplox/core/ValueStack.hpp"

namespace cpplox {
    enum class InterpretResult {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    class VM {
    public:
        VM() = default;
        ~VM() = default;

        VM(const VM&) = delete;
        VM& operator=(const VM&) = delete;

        InterpretResult interpret(const Chunk& chunk);

    private:
        InterpretResult run();

        template <ValueBinaryOp F>
        void binaryOp(const F& f) {
            Value b = stack.pop();
            const Value& a = stack.peek();
            stack.peek() = f(a, b);
        }

    private:
        const std::uint8_t* ip = nullptr;
        const Chunk* chunk = nullptr;
        ValueStack stack;
    };
} // namespace cpplox