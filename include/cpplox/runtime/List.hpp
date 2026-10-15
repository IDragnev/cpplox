#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/Vector.hpp"
#include "cpplox/core/Value.hpp"

namespace cpplox {
    class List : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::LIST;

        List();
        explicit List(std::size_t size);

        void trace(gc::Visitor& v) override;

        Vector<Value> values;
    };

    bool structurallyEqual(const List* l, const List* r);
}
