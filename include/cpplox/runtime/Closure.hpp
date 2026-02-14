#pragma once

#include "cpplox/runtime/Function.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class Upvalue;

    class Closure : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::CLOSURE;

        explicit Closure(const Function* fun);

        const Function* const function = nullptr;
        Vector<Upvalue*> upvalues;
    };
}