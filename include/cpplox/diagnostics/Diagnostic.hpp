#pragma once

#include <string>

namespace cpplox {
    struct Diagnostic {
        std::string msg;
        std::size_t line = 0;
    };
} // namespace cpplox