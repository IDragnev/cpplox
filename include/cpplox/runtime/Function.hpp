#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/bytecode/Chunk.hpp"

namespace cpplox {
    class Function : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::FUNCTION;

        explicit Function(const String& name);

        void trace(gc::Visitor& v) override;

        const String name;
        unsigned arity = 0;
        unsigned upvaluesCount = 0;
        Chunk chunk;
    };
}