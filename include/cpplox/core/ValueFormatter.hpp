#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/core/StringFormatter.hpp"
#include <fmt/format.h>

template <>
struct fmt::formatter<cpplox::Value> {
    fmt::formatter<std::string_view> spec;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return spec.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const cpplox::Value& v, FormatContext& ctx) const {
        switch (v.internalType()) {
            case cpplox::ValueType::NIL: {
                return spec.format(std::string_view("nil"), ctx);
            } break;
            case cpplox::ValueType::BOOL: {
                return fmt::formatter<bool>{}.format(v.asBoolean(), ctx);
            } break;
            case cpplox::ValueType::NUMBER: {
                return fmt::formatter<double>{}.format(v.asNumber(), ctx);
            } break;
            case cpplox::ValueType::STRING: {
                std::string s = fmt::format("\"{}\"", v.asString());
                return spec.format(std::string_view(s), ctx);
            } break;
            case cpplox::ValueType::OBJECT: {
                std::string s =
                    fmt::format("<obj {}>",
                                static_cast<const void*>(v.asObject()));
                return spec.format(std::string_view(s), ctx);
            } break;
        }

        return spec.format(std::string_view("<invalid>"), ctx);
    }
};