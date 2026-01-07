#pragma once

#include <cstdint>

#include "cpplox/core/Value.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    struct Chunk {
        Vector<std::uint8_t> code;
        Vector<unsigned> lines;
        Vector<Value> constants;
    };
} // namespace cpplox