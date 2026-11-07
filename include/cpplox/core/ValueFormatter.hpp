#pragma once

#include "cpplox/core/Value.hpp"
#include <fmt/format.h>

template <>
struct fmt::formatter<cpplox::Value> {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
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
            case cpplox::ValueType::OBJECT: {
                return fmt::format_to(ctx.out(),
                                      "<obj {}>",
                                      static_cast<const void*>(v.asObject()));
            } break;
        }

        return fmt::format_to(ctx.out(), "<invalid>");
    }
};
