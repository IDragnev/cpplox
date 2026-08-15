#include "cpplox/runtime/Natives.hpp"
#include "cpplox/log/Log.hpp"
#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/Function.hpp"
#include "cpplox/runtime/Closure.hpp"
#include "cpplox/runtime/Class.hpp"
#include "cpplox/runtime/Instance.hpp"
#include "cpplox/runtime/BoundMethod.hpp"
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
        if (args.size() != 1) {
            return false;
        }

        const Value& v = args[0];
        if (v.isObject() == false) {
            println("{}", v);
        } else {
            const Object* obj = v.asObject();
            switch (obj->type()) {
                case ObjectType::FUNCTION: {
                    const Function* fun = obj->as<Function>();
                    println("<fun {}:{}>", fun->name, fun->arity);
                } break;
                case ObjectType::CLASS: {
                    const Class* classObj = obj->as<Class>();
                    println("<class {}>", classObj->name);
                } break;
                case ObjectType::INSTANCE: {
                    const Instance* instance = obj->as<Instance>();
                    println("<{} intance>", instance->klass->name);
                } break;
                case ObjectType::CLOSURE: {
                    const Closure* closure = obj->as<Closure>();
                    println("<fun {}:{}>",
                            closure->function->name,
                            closure->function->arity);
                } break;
                case ObjectType::BOUND_METHOD: {
                    const BoundMethod* method = obj->as<BoundMethod>();
                    println("<fun {}:{}>",
                            method->method->function->name,
                            method->method->function->arity);
                } break;
                case ObjectType::NATIVE_FUNCTION: {
                    const NativeFunction* fun = obj->as<NativeFunction>();
                    println("<native fun {}:{}>", fun->name, fun->arity);
                } break;
                case ObjectType::UPVALUE: { } break;
            }
        }

        return true;
    }
} // namespace cpplox
