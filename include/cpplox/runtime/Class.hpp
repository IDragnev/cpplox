#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"

namespace cpplox {
    class Class : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::CLASS;

        explicit Class(const String& name);

        void trace(gc::Visitor& v) override;

        const String name;
    };
}