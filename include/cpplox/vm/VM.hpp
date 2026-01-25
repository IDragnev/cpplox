#pragma once

#include "cpplox/core/ValueStack.hpp"
#include "cpplox/core/ValueMap.hpp"
#include "cpplox/core/Vector.hpp"
#include "cpplox/core/String.hpp"

namespace cpplox {
    class Function;
    class Closure;
    class Object;

    enum class InterpretResultCode {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    struct InterpretResult {
        InterpretResultCode code = InterpretResultCode::OK;
        String error = "";
    };

    template <typename Op>
    concept NumberBinaryOp =
        requires(Op op, double a, double b, Value c) { c = op(a, b); };

    class VM {
        struct CallFrame {
            const Closure* closure = nullptr;
            const std::uint8_t* ip = nullptr;
            std::size_t bp = 0;
        };

    public:
        VM() = default;
        ~VM();

        VM(const VM&) = delete;
        VM& operator=(const VM&) = delete;

        InterpretResult interpret(Function* func, Vector<Object*>&& objects);

    private:
        InterpretResultCode run();
        void addObjects(Vector<Object*>&& objects);
        template <typename T, typename... Args>
        T* makeObject(Args&&... args);

        template <NumberBinaryOp Op>
        bool numBinaryOp(const Op& op);

        bool callValue(const Value& v, std::uint8_t argc);
        bool call(const Closure* f, std::uint8_t argc);
        void printValue(const Value& v) const;

        template <typename... Args>
        void runtimeError(std::string_view fmtStr, Args&&... args);
        void appendCallStackInfo(struct FmtBuffer& buf);

    private:
        ValueStack stack;
        ValueMap globals;
        Vector<CallFrame> frames;
        Vector<Object*> gcObjects;
        String error = "";
    };
} // namespace cpplox