#pragma once

#include "cpplox/core/Terminal.hpp"

#include <fmt/base.h>
#include <fmt/color.h>

namespace cpplox {
    template <typename... Args>
    void print(fmt::format_string<Args...> fmtStr, Args&&... args) {
        fmt::print(stdout, fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void println(fmt::format_string<Args...> fmtStr, Args&&... args) {
        cpplox::print(fmtStr, std::forward<Args>(args)...);
        cpplox::print("\n");
    }

    template <typename... Args>
    void error(fmt::format_string<Args...> fmtStr, Args&&... args) {
        if (STDERR_IS_TERMINAL) {
            fmt::print(stderr, fg(fmt::color::red), fmtStr, std::forward<Args>(args)...);
        } else {
            fmt::print(stderr, fmtStr, std::forward<Args>(args)...);
        }
    }
    template <typename... Args>
    void errorln(fmt::format_string<Args...> fmtStr, Args&&... args) {
        cpplox::error(fmtStr, std::forward<Args>(args)...);
        cpplox::error("\n");
    }
} // namespace cpplox