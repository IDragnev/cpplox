#include "cpplox/core/Value.hpp"

namespace cpplox {
    bool Value::operator==(const Value& rhs) const {
        if (type != rhs.type) {
            return false;
        }

        switch (type) {
            case ValueType::BOOL: {
                return asBoolean() == rhs.asBoolean();
            } break;
            case ValueType::NUMBER: {
                return asNumber() == rhs.asNumber();
            } break;
            case ValueType::OBJECT: {
                return asObject() == rhs.asObject();
            } break;
            case ValueType::NIL: {
                return true;
            } break;
        }

        return false;
    }

    bool Value::isFalsey() const {
        return isNil() || (isBoolean() && (asBoolean() == false));
    }
} // namespace cpplox