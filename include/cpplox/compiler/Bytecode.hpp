#pragma once

#include <cstdint>
#include "cpplox/compiler/Chunk.hpp"

namespace cpplox {
    bool fitsOneByte(std::size_t i);
    bool fitsTwoBytes(std::size_t i);
    void serializeTwoByteInteger(std::size_t i, std::uint8_t& a, std::uint8_t& b);
    std::size_t parseTwoByteInteger(std::uint8_t a, std::uint8_t b);

    void addCode(Chunk& chunk, std::uint8_t c, unsigned l);
    std::size_t addConstant(Chunk& chunk, Value&& v);
} // namespace cpplox