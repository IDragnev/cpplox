#pragma once

#include <cstdint>

#include "cpplox/core/Value.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    enum class OpCode {
        RETURN,
        CONSTANT,
        CONSTANT_16,
        NIL,
        TRUE,
        FALSE,
        NOT,
        NEGATE,
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
    };

    struct Chunk {
        Vector<std::uint8_t> code;
        Vector<unsigned> lines;
        Vector<Value> constants;
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
        chunk.code.insertBack(c);
        chunk.lines.insertBack(l);
    }

    inline std::size_t addConstant(Chunk& chunk, Value&& v) {
        chunk.constants.insertBack(std::move(v));
        return chunk.constants.getCount() - 1;
    }
} // namespace cpplox