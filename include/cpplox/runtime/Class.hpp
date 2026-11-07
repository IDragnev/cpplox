#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/RuntimeStr.hpp"
#include "cpplox/runtime/ValueMap.hpp"

namespace cpplox {
    class Class : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::CLASS;

        explicit Class(RuntimeStr* name);

        void trace(gc::Visitor& v) override;

        RuntimeStr* const name = nullptr;
        ValueMap methods;

    protected:
        std::size_t computeAllocSize() const override;
    };
}