#pragma once

#include "cpplox/runtime/Object.hpp"

namespace cpplox {
    class Value;

    class Upvalue : public Object {
    public:
        static const ObjectType TYPE = ObjectType::UPVALUE;

        explicit Upvalue(Value* v) : Object(TYPE), location(v) {}

        Value closed;
        Value* location = nullptr;
        Upvalue* next = nullptr;
    };
} // namespace cpplox