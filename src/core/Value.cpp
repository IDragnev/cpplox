#include "cpplox/core/Value.hpp"
#include <stdio.h>

namespace cpplox {
    bool operator==(const Value& a, const Value& b) {
        if (a.type != b.type) {
            return false;
        }

        switch (a.type) {
            case ValueType::BOOL: {
                return value::asBool(a) == value::asBool(b);
            } break;
            case ValueType::NUMBER: {
                return value::asNumber(a) == value::asNumber(b);
            } break;
            case ValueType::NIL: {
                return true;
            } break;
            default: {
                return false;
            } break;
        }
    }

    namespace value {
        bool isFalsey(const Value& v) {
            return isNil(v) || (v == value::FALSE);
        }

        void print(const Value& v) {
            switch (v.type) {
                case ValueType::BOOL: {
                    printf(asBool(v) ? "true" : "false");
                } break;
                case ValueType::NUMBER: {
                    printf("%g", asNumber(v));
                } break;
                case ValueType::NIL: {
                    printf("nil");
                } break;
            }
        }
    } // namespace value
} // namespace cpplox