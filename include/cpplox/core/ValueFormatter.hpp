#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/core/StringFormatter.hpp"
#include <fmt/format.h>

template <>
struct fmt::formatter<cpplox::Value> {
    bool debug = false;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == '?') {
            debug = true;
            ++it;
        }
        if (it != ctx.end() && *it != '}') {
            throw fmt::format_error("invalid format spec for Value");
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const cpplox::Value& v, FormatContext& ctx) const {
        switch (v.internalType()) {
            case cpplox::ValueType::NIL: {
                return fmt::format_to(ctx.out(), "nil");
            } break;
            case cpplox::ValueType::BOOL: {
                return fmt::format_to(ctx.out(), "{}", v.asBoolean());
            } break;
            case cpplox::ValueType::NUMBER: {
                return fmt::format_to(ctx.out(), "{}", v.asNumber());
            } break;
            case cpplox::ValueType::STRING: {
                return debug ? fmt::format_to(ctx.out(), "{:?}", v.asString())
                             : fmt::format_to(ctx.out(), "{}", v.asString());
            } break;
            case cpplox::ValueType::OBJECT: {
                return fmt::format_to(ctx.out(),
                                      "<obj {}>",
                                      static_cast<const void*>(v.asObject()));
            } break;
        }

        return fmt::format_to(ctx.out(), "<invalid>");
    }
};
