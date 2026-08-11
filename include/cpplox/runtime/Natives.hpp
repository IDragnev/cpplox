#pragma once

#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    class Clock : public NativeFunction {
    public:
        Clock() : NativeFunction("clock", 0) {}

        bool call(std::span<Value> args, Value& result) override;
    };
}
