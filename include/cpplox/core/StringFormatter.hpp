#pragma once

#include "cpplox/core/String.hpp"
#include <fmt/format.h>

template <>
struct fmt::formatter<cpplox::String> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const cpplox::String& s, FormatContext& ctx) const {
        return fmt::formatter<std::string_view>::format(s.view(), ctx);
    }
};