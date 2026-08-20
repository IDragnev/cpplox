#include "cpplox/runtime/Natives.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/ObjectFormatter.hpp"
#include "cpplox/log/Log.hpp"
#include "cpplox/core/Format.hpp"

#include <chrono>
#include <string_view>

namespace cpplox {
    namespace {
        using Clk = std::chrono::steady_clock;
        const Clk::time_point programStart = Clk::now();

        String renderValue(const Value& v) {
            if (v.isObject()) {
                return String(fmt::format("{}", *v.asObject()));
            }

            return String(fmt::format("{}", v));
        }

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
                case ObjectType::UPVALUE: break;
            }

            // Upvalues are internal and never reachable from a script.
            return "unknown";
        }
    } // namespace

    bool Clock::call(std::span<Value>, Value& result) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clk::now() - programStart);
        // Whole milliseconds, widened to a double only because that is the
        // single number type cpplox has.
        result = Value(static_cast<double>(elapsed.count()));

        return true;
    }

    bool Print::call(std::span<Value> args, Value&) {
        if (args.empty()) {
            println("");
            return true;
        }

        for (const Value& v : args) {
            println("{}", renderValue(v));
        }

        return true;
    }

    bool Str::call(std::span<Value> args, Value& result) {
        result = Value(renderValue(args[0]));

        return true;
    }

    bool Type::call(std::span<Value> args, Value& result) {
        result = Value(typeName(args[0]));

        return true;
    }

    bool Assert::call(std::span<Value> args, Value& result) {
        if (args[0].isFalsey() == false) {
            return true;
        }

        if (args.size() == 2) {
            result = args[1].isString() ? args[1]
                                        : Value(renderValue(args[1]));
        } else {
            result = Value(String("Assertion failed."));
        }

        return false;
    }
} // namespace cpplox
