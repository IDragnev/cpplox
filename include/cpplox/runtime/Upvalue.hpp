#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/Value.hpp"

namespace cpplox {
    class Upvalue : public Object {
    public:
        static const ObjectType TYPE = ObjectType::UPVALUE;

        explicit Upvalue(Value* v) : Object(TYPE), location(v) {}

        void trace(gc::Visitor& v) override;

        Value closed;
        Value* location = nullptr;
        Upvalue* next = nullptr;

    protected:
        std::size_t computeAllocSize() const override { return sizeof(Upvalue); }
    };
} // namespace cpplox