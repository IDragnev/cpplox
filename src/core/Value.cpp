#include "cpplox/core/Value.hpp"
#include <stdio.h>

namespace cpplox {
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
            case ValueType::NIL: {
                return true;
            } break;
        }

        return false;
    }

    bool Value::isFalsey() const {
        return isNil() || (isBoolean() && (asBoolean() == false));
    }

    void Value::print() const {
        switch (type) {
            case ValueType::BOOL: {
                printf(asBoolean() ? "true" : "false");
            } break;
            case ValueType::NUMBER: {
                printf("%g", asNumber());
            } break;
            case ValueType::NIL: {
                printf("nil");
            } break;
            case ValueType::STRING: {
                printf("%s", asString().c_str());
            } break;
        }
    }
} // namespace cpplox