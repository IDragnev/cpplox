#include "cpplox/core/Value.hpp"
#include "cpplox/core/String.hpp"

namespace cpplox {
    Value::Value(String&& s)
        : type(ValueType::STRING)
        , as{.string = new String(std::move(s))}
    {}

    Value::Value(std::string_view s)
        : type(ValueType::STRING)
        , as{.string = new String(s)}
    {}

    Value& Value::operator=(const Value& other) {
        if (this != &other) {
            destroy();
            copy(other);
        }

        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            destroy();
            move(std::move(other));
        }

        return *this;
    }

    void Value::move(Value&& other) {
        type = other.type;
        switch (other.type) {
            case ValueType::NIL: {
            } break;
            case ValueType::BOOL: {
                this->as.boolean = other.as.boolean;
            } break;
            case ValueType::NUMBER: {
                this->as.number = other.as.number;
            } break;
            case ValueType::STRING: {
                this->as.string = other.as.string;
            } break;
            case ValueType::OBJECT: {
                this->as.object = other.as.object;
            } break;
        }

        other.type = ValueType::NIL;
        other.as.boolean = false;
    }

    void Value::copy(const Value& other) {
        type = other.type;
        switch (other.type) {
            case ValueType::NIL: {
            } break;
            case ValueType::BOOL: {
                this->as.boolean = other.as.boolean;
            } break;
            case ValueType::NUMBER: {
                this->as.number = other.as.number;
            } break;
            case ValueType::STRING: {
                this->as.string = new String(*other.as.string);
            } break;
            case ValueType::OBJECT: {
                this->as.object = other.as.object;
            } break;
        }
    }

    void Value::destroy() {
        if (type == ValueType::STRING) {
            delete (this->as.string);
        }
    }

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
            case ValueType::STRING: {
                return asString() == rhs.asString();
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