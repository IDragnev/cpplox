#pragma once

#include <cstdint>
#include <vector>

#include "cpplox/core/Value.hpp"

namespace cpplox {
    enum class OpCode {
        RETURN,
        CONSTANT,
        CONSTANT_16,
        NEGATE,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
    };

    struct Chunk {
        std::vector<std::uint8_t> code;
        std::vector<unsigned> lines;
        std::vector<Value> constants;
    };

    inline void
    serializeConstant16Index(std::size_t i, std::uint8_t& a, std::uint8_t& b) {
        a = static_cast<std::uint8_t>(i & 0xFF);
        b = static_cast<std::uint8_t>((i & 0xFF00) >> 8);
    }

    inline std::size_t parseConstant16Index(std::uint8_t a, std::uint8_t b) {
        std::size_t i = static_cast<std::size_t>(b) << 8;
        i = i | static_cast<std::size_t>(a);
        return i;
    }

    inline void addCode(Chunk& chunk, std::uint8_t c, unsigned l) {
        chunk.code.push_back(c);
        chunk.lines.push_back(l);
    }

    inline std::size_t addConstant(Chunk& chunk, const Value& v) {
        chunk.constants.push_back(v);
        return chunk.constants.size() - 1;
    }
} // namespace cpplox