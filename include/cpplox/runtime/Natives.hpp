#pragma once

#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    class Clock : public NativeFunction {
    public:
        Clock() : NativeFunction("clock", Arity::exactly(0)) {}

        bool call(std::span<Value> args, Value& result) override;
    };

    class Print : public NativeFunction {
    public:
        Print() : NativeFunction("print", Arity::any()) {}

        bool call(std::span<Value> args, Value& result) override;
    };

    class Str : public NativeFunction {
    public:
        Str() : NativeFunction("str", Arity::exactly(1)) {}

        bool call(std::span<Value> args, Value& result) override;
    };

    class Type : public NativeFunction {
    public:
        Type() : NativeFunction("type", Arity::exactly(1)) {}

        bool call(std::span<Value> args, Value& result) override;
    };

    class Assert : public NativeFunction {
    public:
        Assert() : NativeFunction("assert", Arity::between(1, 2)) {}

        bool call(std::span<Value> args, Value& result) override;
    };
}
