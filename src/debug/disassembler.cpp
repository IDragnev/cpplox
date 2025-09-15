#include "cpplox/debug/Disassembler.hpp"
#include <cassert>

namespace cpplox::debug {
    void Disassembler::disassembleChunk(const Chunk& chunk,
                                        const std::string_view& name) const {
        printf("=== %s ===\n", name.data());

        for (std::size_t offset = 0; offset < chunk.code.size();) {
            offset = disassembleInstruction(chunk, offset);
        }
    }

    std::size_t Disassembler::disassembleInstruction(const Chunk& chunk,
                                                     std::size_t offset) const {
        if (offset >= chunk.code.size()) {
            return offset;
        }

        printf("%04zu ", offset);
        if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
            printf("   | ");
        } else {
            printf("%4u ", chunk.lines[offset]);
        }

        const std::uint8_t opCode = chunk.code[offset];
        switch (static_cast<OpCode>(opCode)) {
            case OpCode::ADD: {
                return simpleInstruction("ADD", offset);
            } break;
            case OpCode::SUBTRACT: {
                return simpleInstruction("SUBTRACT", offset);
            } break;
            case OpCode::DIVIDE: {
                return simpleInstruction("DIVIDE", offset);
            } break;
            case OpCode::MULTIPLY: {
                return simpleInstruction("MULTIPLY", offset);
            } break;
            case OpCode::NEGATE: {
                return simpleInstruction("NEGATE", offset);
            } break;
            case OpCode::RETURN: {
                return simpleInstruction("RETURN", offset);
            } break;
            case OpCode::CONSTANT: {
                return constantInstruction("CONSTANT", chunk, offset);
            } break;
            default: {
                printf("Unknown opcode %d\n", opCode);
                return offset + 1;
            } break;
        }
    }

    std::size_t Disassembler::simpleInstruction(const char* name,
                                                std::size_t offset) const {
        printf("%s\n", name);
        return offset + 1;
    }

    std::size_t Disassembler::constantInstruction(const char* name,
                                                  const Chunk& chunk,
                                                  std::size_t offset) const {
        auto constant = chunk.code[offset + 1];

        printf("%-16s %4d '", name, constant);
        printValue(chunk.constants[constant]);
        printf("'\n");

        return offset + 2;
    }
} // namespace cpplox::debug