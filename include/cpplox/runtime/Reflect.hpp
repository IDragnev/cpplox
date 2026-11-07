#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/runtime/Object.hpp"

#include <string_view>

namespace cpplox {
    inline bool isObjectOfType(const Value& v, ObjectType t) {
        return v.isObject() && v.asObject()->hasType(t);
    }

    inline bool isList(const Value& v) {
        return isObjectOfType(v, ObjectType::LIST);
    }

    inline bool isString(const Value& v) {
        return isObjectOfType(v, ObjectType::STRING);
    }

    std::string_view typeName(const Value& v);

    // Strings are not interned yet, so they are compared by content. Other
    // objects are compared by identity.
    bool valuesEqual(const Value& a, const Value& b);
} // namespace cpplox
