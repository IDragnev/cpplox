#pragma once

#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    class Clock : public NativeFunction {
    public:
        Clock() : NativeFunction("clock", Arity::exactly(0)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Print : public NativeFunction {
    public:
        Print() : NativeFunction("print", Arity::any()) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Str : public NativeFunction {
    public:
        Str() : NativeFunction("str", Arity::exactly(1)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Type : public NativeFunction {
    public:
        Type() : NativeFunction("type", Arity::exactly(1)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Assert : public NativeFunction {
    public:
        Assert() : NativeFunction("assert", Arity::between(1, 2)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Push : public NativeFunction {
    public:
        Push() : NativeFunction("push", Arity::exactly(2)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Pop : public NativeFunction {
    public:
        Pop() : NativeFunction("pop", Arity::exactly(1)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Len : public NativeFunction {
    public:
        Len() : NativeFunction("len", Arity::exactly(1)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class ListEquals : public NativeFunction {
    public:
        ListEquals() : NativeFunction("listEquals", Arity::exactly(2)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };

    class Floor : public NativeFunction {
    public:
        Floor() : NativeFunction("floor", Arity::exactly(1)) {}

        bool call(NativeContext& ctx, std::span<Value> args, Value& result) override;
    };
}
