#pragma once

#include <string_view>
#include "cpplox/core/Chunk.hpp"

namespace cpplox::debug {
    class Disassembler {
    public:
        void disassembleChunk(const Chunk& chunk,
                              const std::string_view& name) const;

        std::size_t disassembleInstruction(const Chunk& chunk,
                                            std::size_t offset) const;
    private:
        std::size_t simpleInstruction(const char* name,
                                      std::size_t offset) const;
        std::size_t constantInstruction(const char* name,
                                        const Chunk& chunk,
                                        std::size_t offset) const;
        std::size_t constant16Instruction(const char* name,
                                          const Chunk& chunk,
                                          std::size_t offset) const;
    };
} // namespace cpplox::debug