#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/RuntimeStr.hpp"
#include "cpplox/bytecode/Chunk.hpp"

namespace cpplox {
    class Function : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::FUNCTION;

        Function(RuntimeStr* name, RuntimeStr* source);

        void trace(gc::Visitor& v) override;

        RuntimeStr* const name = nullptr;
        RuntimeStr* const source = nullptr;
        unsigned arity = 0;
        unsigned upvaluesCount = 0;
        Chunk chunk;

    protected:
        std::size_t computeAllocSize() const override;
    };
}