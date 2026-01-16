#include "cpplox/bytecode/Bytecode.hpp"

namespace cpplox {
    bool fitsOneByte(std::size_t i) {
        return i <= static_cast<std::size_t>(0xFF);
    }

    bool fitsTwoBytes(std::size_t i) {
        return i <= static_cast<std::size_t>(0xFFFF);
    }

    void serializeTwoByteInteger(std::size_t i, std::uint8_t& a, std::uint8_t& b) {
        a = static_cast<std::uint8_t>(i & 0xFF);
        b = static_cast<std::uint8_t>((i & 0xFF00) >> 8);
    }

    std::size_t parseTwoByteInteger(std::uint8_t a, std::uint8_t b) {
        std::size_t i = static_cast<std::size_t>(b) << 8;
        i = i | static_cast<std::size_t>(a);
        return i;
    }

    void addCode(Chunk& chunk, std::uint8_t c, unsigned l) {
        chunk.code.insertBack(c);
        chunk.lines.insertBack(l);
    }

    std::size_t addConstant(Chunk& chunk, Value&& v) {
        chunk.constants.insertBack(std::move(v));
        return chunk.constants.getCount() - 1;
    }
}