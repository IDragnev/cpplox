#pragma once

#include "cpplox/runtime/Function.hpp"

namespace cpplox {
    class Closure : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::CLOSURE;

        explicit Closure(const Function* fun);

        const Function* const function = nullptr;
    };
}