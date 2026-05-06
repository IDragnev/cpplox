#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/ValueMap.hpp"

namespace cpplox {
    class Class;

    class Instance : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::INSTANCE;

        explicit Instance(Class* c);

        void trace(gc::Visitor& v) override;

        Class* const klass = nullptr;
        ValueMap fields;
    };
}