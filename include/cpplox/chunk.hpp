#pragma once

#include <stdint.h>
#include <vector>

namespace cpplox {
    enum class OpCode {
        RETURN,
        CONSTANT,
    };

    using Value = double;

    struct Chunk {
        std::vector<std::uint8_t> code;
        std::vector<unsigned> lines;
        std::vector<Value> constants;
    };

    inline void addCode(Chunk& chunk, std::uint8_t c, unsigned l) {
        chunk.code.push_back(c);
        chunk.lines.push_back(l);
    }

    inline std::size_t addConstant(Chunk& chunk, const Value& v) {
        chunk.constants.push_back(v);
        return chunk.constants.size() - 1;
    }
} // namespace cpplox