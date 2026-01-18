#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/bytecode/Chunk.hpp"

namespace cpplox {
    class Function : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::FUNCTION;

        explicit Function(const String& name);
        Function(const String& name, unsigned arity);
        Function(const String& name, unsigned arity, const Chunk& c);

        Function* clone() const override;

        const String name;
        unsigned arity = 0;
        Chunk chunk;
    };
}