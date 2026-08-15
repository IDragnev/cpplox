#include "cpplox/runtime/Natives.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/ObjectFormatter.hpp"
#include "cpplox/log/Log.hpp"
#include "cpplox/core/Format.hpp"

#include <chrono>

namespace cpplox {
    namespace {
        using Clk = std::chrono::steady_clock;
        const Clk::time_point programStart = Clk::now();
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
            if (v.isObject()) {
                println("{}", *v.asObject());
            } else {
                println("{}", v);
            }
        }

        return true;
    }
} // namespace cpplox
