#include "cpplox/runtime/Reflect.hpp"

namespace cpplox {
    std::string_view typeName(const Value& v) {
        switch (v.internalType()) {
            case ValueType::NIL: return "nil";
            case ValueType::BOOL: return "bool";
            case ValueType::NUMBER: return "number";
            case ValueType::STRING: return "string";
            case ValueType::OBJECT: break;
        }

        switch (v.asObject()->type()) {
            case ObjectType::FUNCTION:
            case ObjectType::CLOSURE:
            case ObjectType::BOUND_METHOD: return "function";
            case ObjectType::NATIVE_FUNCTION: return "native function";
            case ObjectType::CLASS: return "class";
            case ObjectType::INSTANCE: return "instance";
            case ObjectType::LIST: return "list";
            case ObjectType::UPVALUE: break;
        }

        // Upvalues are internal and never reachable from a script.
        return "unknown";
    }
} // namespace cpplox
