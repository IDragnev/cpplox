#pragma once

#include "cpplox/core/ValueStack.hpp"
#include "cpplox/core/ValueMap.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class Function;
    class Object;

    enum class InterpretResult {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    template <typename Op>
    concept NumberBinaryOp =
        requires(Op op, double a, double b, Value c) { c = op(a, b); };

    class VM {
        struct CallFrame {
            const Function* function = nullptr;
            const std::uint8_t* ip = nullptr;
            std::size_t bp = 0;
        };

    public:
        VM() = default;
        ~VM() = default;

        VM(const VM&) = delete;
        VM& operator=(const VM&) = delete;

        InterpretResult interpret(Function* func, Vector<Object*>&& objects);

    private:
        InterpretResult run();

        template <NumberBinaryOp Op>
        bool numBinaryOp(const Op& op);

        void runtimeError(const char* msg);

    private:
        ValueStack stack;
        ValueMap globals;
        Vector<CallFrame> frames;
    };
} // namespace cpplox