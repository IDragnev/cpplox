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
}
