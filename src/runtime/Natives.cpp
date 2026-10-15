#include "cpplox/runtime/Natives.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/ObjectFormatter.hpp"
#include "cpplox/log/Log.hpp"
#include "cpplox/core/Format.hpp"
#include "cpplox/runtime/List.hpp"
#include "cpplox/runtime/Reflect.hpp"

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

    bool Push::call(std::span<Value> args, Value& result) {
        const Value& list = args[0];
        if (isList(list) == false) {
            result = Value(String(fmt::format("push() expects a list, got {}.",
                                              typeName(list))));
            return false;
        }

        List* listObj = args[0].asObject()->as<List>();
        listObj->values.insertBack((args[1]));
        
        //grow gc bytes and size if needed

        return true;
    }

    bool Pop::call(std::span<Value> args, Value& result) {
        Value& list = args[0];
        if (isList(list) == false) {
            result = Value(String(fmt::format("pop() expects a list, got {}.",
                                              typeName(list))));
            return false;
        }

        List* listObj = list.asObject()->as<List>();
        if (listObj->values.isEmpty()) {
            result = Value(String("Cannot pop from an empty list."));
            return false;
        }
        
        result = listObj->values.back();
        listObj->values.removeBack();

        //shrink gc bytes and size if needed

        return true;
    }

    bool Len::call(std::span<Value> args, Value& result) {
        Value& list = args[0];
        if (isList(list) == false) {
            result = Value(String(fmt::format("len() expects a list, got {}.",
                                              typeName(list))));
            return false;
        }

        List* listObj = list.asObject()->as<List>();
        result = Value(static_cast<double>(listObj->values.getCount()));

        return true;
    }

    bool ListEquals::call(std::span<Value> args, Value& result) {
        if (isList(args[0]) == false || isList(args[1]) == false) {
            result = Value(String("listEquals() expects two lists."));
            return false;
        }

        const List* list1 = args[0].asObject()->as<List>();
        const List* list2 = args[1].asObject()->as<List>();

        result = Value(structurallyEqual(list1, list2));
        
        return true;
    }

    bool Floor::call(std::span<Value> args, Value& result) {
        if (args[0].isNumber() == false) {
            result = Value(String(fmt::format("floor() expects a number, got {}.",
                                              typeName(args[0]))));
            return false;
        }

        result = Value(std::floor(args[0].asNumber()));

        return true;
    }
} // namespace cpplox
