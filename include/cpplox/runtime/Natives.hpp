#pragma once

#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    class Clock : public NativeFunction {
    public:
        explicit Clock(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(0))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Print : public NativeFunction {
    public:
        explicit Print(RuntimeStr* name)
            : NativeFunction(name, Arity::any())
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Str : public NativeFunction {
    public:
        explicit Str(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(1))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Type : public NativeFunction {
    public:
        explicit Type(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(1))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Assert : public NativeFunction {
    public:
        explicit Assert(RuntimeStr* name)
            : NativeFunction(name, Arity::between(1, 2))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Push : public NativeFunction {
    public:
        explicit Push(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(2))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Pop : public NativeFunction {
    public:
        explicit Pop(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(1))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class MakeList : public NativeFunction {
    public:
        explicit MakeList(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(2))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Len : public NativeFunction {
    public:
        explicit Len(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(1))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class ListEquals : public NativeFunction {
    public:
        explicit ListEquals(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(2))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };

    class Floor : public NativeFunction {
    public:
        explicit Floor(RuntimeStr* name)
            : NativeFunction(name, Arity::exactly(1))
        {}

        Value call(NativeContext& ctx, std::span<Value> args) override;
    };
}
