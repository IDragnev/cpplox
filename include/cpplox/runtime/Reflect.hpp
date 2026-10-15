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

    std::string_view typeName(const Value& v);
} // namespace cpplox
