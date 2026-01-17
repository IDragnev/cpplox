#pragma once

#include <string_view>

namespace cpplox {
    class Object;
    class String;

    enum class ValueType {
        BOOL,
        NIL,
        NUMBER,
        STRING,
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
        explicit Value(String&& s);
        explicit Value(std::string_view s);
        explicit Value(Object* obj)
            : type(ValueType::OBJECT)
            , as{.object = obj}
        {}

        Value(const Value& other) { copy(other); }
        Value(Value&& other) noexcept { move(std::move(other)); }
        ~Value() { destroy(); }

        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        static Value nil() { return Value(); }

        ValueType internalType() const { return type; }
        bool holdsType(ValueType t) const { return type == t; }
        bool isNil() const { return holdsType(ValueType::NIL); }
        bool isBoolean() const { return holdsType(ValueType::BOOL); }
        bool isNumber() const { return holdsType(ValueType::NUMBER); }
        bool isString() const { return holdsType(ValueType::STRING); }
        bool isObject() const { return holdsType(ValueType::OBJECT); }

        bool asBoolean() const { return as.boolean; }
        double asNumber() const { return as.number; }
        const String& asString() const { return *(as.string); }
        String& asString() { return *(as.string); }
        Object* asObject() { return as.object; }
        const Object* asObject() const { return as.object; }

        bool isFalsey() const;

        bool operator==(const Value& rhs) const;
        bool operator!=(const Value& rhs) const {
            return !(*this == rhs);
        }

    private:
        void copy(const Value& other);
        void move(Value&& other);
        void destroy();

    private:
        ValueType type = ValueType::NIL;
        union {
            bool boolean;
            double number;
            String* string;
            Object* object;
        } as;
    };

    static_assert(
        sizeof(Value) <= 16,
        "We should keep Value small enough to be passed around on the stack");
} // namespace cpplox