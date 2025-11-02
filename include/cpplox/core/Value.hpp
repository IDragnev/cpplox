#pragma once

namespace cpplox {
    enum class ValueType {
        BOOL,
        NIL,
        NUMBER,
    };

    struct Value {
        ValueType type = ValueType::NIL;
        union {
            bool boolean;
            double number;
        } as;
    };

    bool operator==(const Value& a, const Value& b);
    inline bool operator!=(const Value& a, const Value& b) {
        return !(a == b);
    }

    namespace value {
        inline constexpr Value boolean(bool b) {
            return Value{.type = ValueType::BOOL, .as{.boolean = b}};
        }

        inline constexpr Value nil() {
            return Value{.type = ValueType::NIL, .as{.number = 0.0}};
        }

        inline Value number(double d) {
            return Value{.type = ValueType::NUMBER, .as{.number = d}};
        }

        static inline constexpr Value TRUE = boolean(true);
        static inline constexpr Value FALSE = boolean(false);
        static inline constexpr Value NIL = nil();

        inline bool asBool(const Value& v) {
            return v.as.boolean;
        }

        inline double asNumber(const Value& v) {
            return v.as.number;
        }

        inline bool isBoolean(const Value& v) {
            return v.type == ValueType::BOOL;
        }

        inline bool isNumber(const Value& v) {
            return v.type == ValueType::NUMBER;
        }

        inline bool isNil(const Value& v) {
            return v.type == ValueType::NIL;
        }

        bool isFalsey(const Value& v);

        void print(const Value& v);
    } // namespace value
} // namespace cpplox