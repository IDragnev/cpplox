#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/runtime/Object.hpp"

namespace cpplox {
    class Closure;

    class BoundMethod : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::BOUND_METHOD;

        BoundMethod(Value receiver, Closure* method);

        void trace(gc::Visitor& v) override;

        Value receiver;
        Closure* const method = nullptr;
    };
}