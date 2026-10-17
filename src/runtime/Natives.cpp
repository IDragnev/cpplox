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

    bool Clock::call(NativeContext&, std::span<Value>, Value& result) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clk::now() - programStart);
        // Whole milliseconds, widened to a double only because that is the
        // single number type cpplox has.
        result = Value(static_cast<double>(elapsed.count()));

        return true;
    }

    bool Print::call(NativeContext&, std::span<Value> args, Value&) {
        if (args.empty()) {
            println("");
            return true;
        }

        for (const Value& v : args) {
            println("{}", display(v));
        }

        return true;
    }

    bool Str::call(NativeContext&, std::span<Value> args, Value& result) {
        result = Value(str(args[0]));

        return true;
    }

    bool Type::call(NativeContext&, std::span<Value> args, Value& result) {
        result = Value(typeName(args[0]));

        return true;
    }

    bool Assert::call(NativeContext&, std::span<Value> args, Value& result) {
        if (args[0].isFalsey() == false) {
            return true;
        }

        if (args.size() == 2) {
            result = Value(str(args[1]));
        } else {
            result = Value(String("Assertion failed."));
        }

        return false;
    }

    bool Push::call(NativeContext& ctx, std::span<Value> args, Value& result) {
        const Value& list = args[0];
        if (isList(list) == false) {
            result = Value(String(fmt::format("push() expects a list, got {}.",
                                              typeName(list))));
            return false;
        }

        List* listObj = args[0].asObject()->as<List>();
        listObj->values.insertBack((args[1]));
        ctx.resized(listObj);

        return true;
    }

    bool Pop::call(NativeContext& ctx, std::span<Value> args, Value& result) {
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
        ctx.resized(listObj);

        return true;
    }

    bool MakeList::call(NativeContext& ctx, std::span<Value> args, Value& result) {
        // Objects are capped at PTRDIFF_MAX bytes, so no allocation can hold
        // more values. Rejecting larger counts also keeps the cast defined.
        constexpr std::size_t MAX_ALLOCATABLE_COUNT =
            static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())
            / sizeof(Value);

        if (args[0].isNumber() == false) {
            result = Value(String(fmt::format(
                "makeList() count must be a number, got {}.", typeName(args[0]))));
            return false;
        }

        const double count = args[0].asNumber();
        if (std::trunc(count) != count) {
            result = Value(String(fmt::format(
                "makeList() count must be a whole number, got {}.", count)));
            return false;
        }
        if (count < 0) {
            result = Value(String(fmt::format(
                "makeList() count must not be negative, got {}.", count)));
            return false;
        }
        if (count > static_cast<double>(MAX_ALLOCATABLE_COUNT)) {
            result = Value(String("Out of memory."));
            return false;
        }

        List* listObj = ctx.newList(static_cast<std::size_t>(count));
        if (listObj == nullptr) {
            result = Value(String("Out of memory."));
            return false;
        }

        forEach(listObj->values, [&args](Value& v) { v = args[1]; });
        result = Value(listObj);

        return true;
    }

    bool Len::call(NativeContext&, std::span<Value> args, Value& result) {
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

    bool ListEquals::call(NativeContext&, std::span<Value> args, Value& result) {
        if (isList(args[0]) == false || isList(args[1]) == false) {
            result = Value(String("listEquals() expects two lists."));
            return false;
        }

        const List* list1 = args[0].asObject()->as<List>();
        const List* list2 = args[1].asObject()->as<List>();

        result = Value(structurallyEqual(list1, list2));
        
        return true;
    }

    bool Floor::call(NativeContext&, std::span<Value> args, Value& result) {
        if (args[0].isNumber() == false) {
            result = Value(String(fmt::format("floor() expects a number, got {}.",
                                              typeName(args[0]))));
            return false;
        }

        result = Value(std::floor(args[0].asNumber()));

        return true;
    }
} // namespace cpplox
