#pragma once

namespace cpplox {
    class Object;

    enum class ValueType {
        BOOL,
        NIL,
        NUMBER,
        OBJECT,
    };

    class Value {
    public:
        Value() = default;
        explicit Value(bool b)
            : type(ValueType::BOOL)
            , as{.boolean = b}
        {}
        explicit Value(double d)
            : type(ValueType::NUMBER)
            , as{.number = d}
        {}
        explicit Value(Object* obj)
            : type(ValueType::OBJECT)
            , as{.object = obj}
        {}

        Value(const Value&) = default;
        Value(Value&&) noexcept = default;
        ~Value() = default;

        Value& operator=(const Value&) = default;
        Value& operator=(Value&&) noexcept = default;

        static Value nil() { return Value(); }

        ValueType internalType() const { return type; }
        bool holdsType(ValueType t) const { return type == t; }
        bool isNil() const { return holdsType(ValueType::NIL); }
        bool isBoolean() const { return holdsType(ValueType::BOOL); }
        bool isNumber() const { return holdsType(ValueType::NUMBER); }
        bool isObject() const { return holdsType(ValueType::OBJECT); }

        bool asBoolean() const { return as.boolean; }
        double asNumber() const { return as.number; }
        Object* asObject() { return as.object; }
        const Object* asObject() const { return as.object; }

        bool isFalsey() const;

        bool operator==(const Value& rhs) const;
        bool operator!=(const Value& rhs) const {
            return !(*this == rhs);
        }

    private:
        ValueType type = ValueType::NIL;
        union {
            bool boolean;
            double number;
            Object* object;
        } as;
    };

    static_assert(
        sizeof(Value) <= 16,
        "We should keep Value small enough to be passed around on the stack");
} // namespace cpplox