#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/bytecode/Chunk.hpp"

namespace cpplox {
    class Function : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::FUNCTION;

        Function(const String& name, const String& source);

        void trace(gc::Visitor& v) override;

        const String name;
        const String source;
        unsigned arity = 0;
        unsigned upvaluesCount = 0;
        Chunk chunk;

    protected:
        std::size_t computeAllocSize() const override;
    };
}