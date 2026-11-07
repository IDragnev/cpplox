#include "cpplox/runtime/Natives.hpp"
#include "cpplox/runtime/NativeContext.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/Display.hpp"
#include "cpplox/log/Log.hpp"
#include "cpplox/core/Format.hpp"
#include "cpplox/runtime/List.hpp"
#include "cpplox/runtime/Reflect.hpp"
#include "cpplox/core/Algorithm.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string_view>

namespace cpplox {
    namespace {
        using Clk = std::chrono::steady_clock;
        const Clk::time_point programStart = Clk::now();
    } // namespace

    Value Clock::call(NativeContext&, std::span<Value>) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clk::now() - programStart);
        // Whole milliseconds, widened to a double only because that is the
        // single number type cpplox has.
        return Value(static_cast<double>(elapsed.count()));
    }

    Value Print::call(NativeContext&, std::span<Value> args) {
        if (args.empty()) {
            println("");
            return Value::nil();
        }

        for (const Value& v : args) {
            println("{}", display(v));
        }

        return Value::nil();
    }

    Value Str::call(NativeContext& ctx, std::span<Value> args) {
        return Value(ctx.newString(str(args[0])));
    }

    Value Type::call(NativeContext& ctx, std::span<Value> args) {
        return Value(ctx.newString(typeName(args[0])));
    }

    Value Assert::call(NativeContext& ctx, std::span<Value> args) {
        if (args[0].isFalsey() == false) {
            return Value::nil();
        }

        if (args.size() == 2) {
            return ctx.error("{}", str(args[1]));
        }

        return ctx.error("Assertion failed.");
    }

    Value Push::call(NativeContext& ctx, std::span<Value> args) {
        const Value& list = args[0];
        if (isList(list) == false) {
            return ctx.error("push() expects a list, got {}.", typeName(list));
        }

        List* listObj = args[0].asObject()->as<List>();
        listObj->values.insertBack((args[1]));
        ctx.resized(listObj);

        return Value::nil();
    }

    Value Pop::call(NativeContext& ctx, std::span<Value> args) {
        Value& list = args[0];
        if (isList(list) == false) {
            return ctx.error("pop() expects a list, got {}.", typeName(list));
        }

        List* listObj = list.asObject()->as<List>();
        if (listObj->values.isEmpty()) {
            return ctx.error("Cannot pop from an empty list.");
        }

        const Value result = listObj->values.back();
        listObj->values.removeBack();
        ctx.resized(listObj);

        return result;
    }

    Value MakeList::call(NativeContext& ctx, std::span<Value> args) {
        // Objects are capped at PTRDIFF_MAX bytes, so no allocation can hold
        // more values. Rejecting larger counts also keeps the cast defined.
        constexpr std::size_t MAX_ALLOCATABLE_COUNT =
            static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())
            / sizeof(Value);

        if (args[0].isNumber() == false) {
            return ctx.error("makeList() count must be a number, got {}.",
                             typeName(args[0]));
        }

        const double count = args[0].asNumber();
        if (std::trunc(count) != count) {
            return ctx.error("makeList() count must be a whole number, got {}.",
                             count);
        }
        if (count < 0) {
            return ctx.error("makeList() count must not be negative, got {}.",
                             count);
        }
        if (count > static_cast<double>(MAX_ALLOCATABLE_COUNT)) {
            return ctx.outOfMemory();
        }

        List* listObj = ctx.newList(static_cast<std::size_t>(count));
        if (listObj == nullptr) {
            return Value::nil();
        }

        forEach(listObj->values, [&args](Value& v) { v = args[1]; });

        return Value(listObj);
    }

    Value Len::call(NativeContext& ctx, std::span<Value> args) {
        Value& list = args[0];
        if (isList(list) == false) {
            return ctx.error("len() expects a list, got {}.", typeName(list));
        }

        List* listObj = list.asObject()->as<List>();

        return Value(static_cast<double>(listObj->values.getCount()));
    }

    Value ListEquals::call(NativeContext& ctx, std::span<Value> args) {
        if (isList(args[0]) == false || isList(args[1]) == false) {
            return ctx.error("listEquals() expects two lists.");
        }

        const List* list1 = args[0].asObject()->as<List>();
        const List* list2 = args[1].asObject()->as<List>();

        return Value(structurallyEqual(list1, list2));
    }

    Value Floor::call(NativeContext& ctx, std::span<Value> args) {
        if (args[0].isNumber() == false) {
            return ctx.error("floor() expects a number, got {}.",
                             typeName(args[0]));
        }

        return Value(std::floor(args[0].asNumber()));
    }
} // namespace cpplox
