#pragma once

#include "cpplox/core/String.hpp"
#include <cstddef>

namespace cpplox {
    struct Diagnostic {
        String msg;
        std::size_t line = 0;
    };
} // namespace cpplox