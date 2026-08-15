#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/Function.hpp"
#include "cpplox/runtime/Closure.hpp"
#include "cpplox/runtime/Class.hpp"
#include "cpplox/runtime/Instance.hpp"
#include "cpplox/runtime/BoundMethod.hpp"
#include "cpplox/runtime/NativeFunction.hpp"
#include "cpplox/core/StringFormatter.hpp"

#include <fmt/format.h>
#include <string>
#include <string_view>

template <>
struct fmt::formatter<cpplox::Object> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const cpplox::Object& obj, FormatContext& ctx) const {
        std::string text;

        switch (obj.type()) {
            case cpplox::ObjectType::FUNCTION: {
                const auto* fun = obj.as<cpplox::Function>();
                text = fmt::format("<fun {}:{}>", fun->name, fun->arity);
            } break;
            case cpplox::ObjectType::CLOSURE: {
                const auto* closure = obj.as<cpplox::Closure>();
                text = fmt::format("<fun {}:{}>",
                                   closure->function->name,
                                   closure->function->arity);
            } break;
            case cpplox::ObjectType::BOUND_METHOD: {
                const auto* method = obj.as<cpplox::BoundMethod>();
                text = fmt::format("<fun {}:{}>",
                                   method->method->function->name,
                                   method->method->function->arity);
            } break;
            case cpplox::ObjectType::CLASS: {
                const auto* classObj = obj.as<cpplox::Class>();
                text = fmt::format("<class {}>", classObj->name);
            } break;
            case cpplox::ObjectType::INSTANCE: {
                const auto* instance = obj.as<cpplox::Instance>();
                text = fmt::format("<{} instance>", instance->klass->name);
            } break;
            case cpplox::ObjectType::NATIVE_FUNCTION: {
                const auto* fun = obj.as<cpplox::NativeFunction>();
                text = fun->arity.isAny()
                           ? fmt::format("<native fun {}:*>", fun->name)
                           : fmt::format("<native fun {}:{}>",
                                         fun->name,
                                         fun->arity.exact());
            } break;
            case cpplox::ObjectType::UPVALUE: {
                // Internal - never reachable from a user-visible value.
                text = "<upvalue>";
            } break;
        }

        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
