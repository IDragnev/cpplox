#include "cpplox/bytecode/Disassembler.hpp"
#include "cpplox/bytecode/OpCode.hpp"
#include "cpplox/bytecode/Bytecode.hpp"
#include "cpplox/core/ValueFormatter.hpp"
#include "cpplox/log/Log.hpp"

namespace cpplox {
    void Disassembler::disassembleChunk(const Chunk& chunk,
                                        const std::string_view& name) const {
        println("=== {} START ===", name);
        for (std::size_t offset = 0; offset < chunk.code.getCount();) {
            offset = disassembleInstruction(chunk, offset);
        }
        println("=== {} END ===", name);
    }

    std::size_t Disassembler::disassembleInstruction(const Chunk& chunk,
                                                     std::size_t offset) const {
        if (offset >= chunk.code.getCount()) {
            return offset;
        }

        constexpr auto printPrefix = [] (auto offset, auto line) {
            constexpr int OFFSET_WIDTH = 4;
            print("{:0{}}    {:>2} ", offset, OFFSET_WIDTH, line);
        };
        if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
            printPrefix(offset, "|");
        } else {
            printPrefix(offset, chunk.lines[offset]);
        }

        const auto opCode = static_cast<OpCode>(chunk.code[offset]);
        switch (opCode) {
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
            case OpCode::NOT: {
                return simpleInstruction("NOT", offset);
            } break;
            case OpCode::EQUAL: {
                return simpleInstruction("EQUAL", offset);
            } break;
            case OpCode::NOT_EQUAL: {
                return simpleInstruction("NOT_EQUAL", offset);
            } break;
            case OpCode::LESS: {
                return simpleInstruction("LESS", offset);
            } break;
            case OpCode::LESS_EQUAL: {
                return simpleInstruction("LESS_EQUAL", offset);
            } break;
            case OpCode::GREATER: {
                return simpleInstruction("GREATER", offset);
            } break;
            case OpCode::GREATER_EQUAL: {
                return simpleInstruction("GREATER_EQUAL", offset);
            } break;
            case OpCode::RETURN: {
                return simpleInstruction("RETURN", offset);
            } break;
            case OpCode::CONSTANT: {
                return constantInstruction("CONSTANT", chunk, offset);
            } break;
            case OpCode::CONSTANT_16: {
                return constant16Instruction("CONSTANT_16", chunk, offset);
            } break;
            case OpCode::DEFINE_GLOBAL: {
                return constantInstruction("DEFINE_GLOBAL", chunk, offset);
            } break;
            case OpCode::DEFINE_GLOBAL_16: {
                return constant16Instruction("DEFINE_GLOBAL_16", chunk, offset);
            } break;
            case OpCode::READ_GLOBAL: {
                return constantInstruction("READ_GLOBAL", chunk, offset);
            } break;
            case OpCode::READ_GLOBAL_16: {
                return constant16Instruction("READ_GLOBAL_16", chunk, offset);
            } break;
            case OpCode::SET_GLOBAL: {
                return constantInstruction("SET_GLOBAL", chunk, offset);
            } break;
            case OpCode::SET_GLOBAL_16: {
                return constant16Instruction("SET_GLOBAL_16", chunk, offset);
            } break;
            case OpCode::READ_LOCAL: {
                return integerInstruction("READ_LOCAL", chunk, offset);
            } break;
            case OpCode::READ_LOCAL_16: {
                return integer16Instruction("READ_LOCAL_16", chunk, offset);
            } break;
            case OpCode::SET_LOCAL: {
                return integerInstruction("SET_LOCAL", chunk, offset);
            } break;
            case OpCode::SET_LOCAL_16: {
                return integer16Instruction("SET_LOCAL_16", chunk, offset);
            } break;
            case OpCode::TRUE: {
                return simpleInstruction("TRUE", offset);
            } break;
            case OpCode::FALSE: {
                return simpleInstruction("FALSE", offset);
            } break;
            case OpCode::NIL: {
                return simpleInstruction("NIL", offset);
            } break;
            case OpCode::PRINT: {
                return simpleInstruction("PRINT", offset);
            } break;
            case OpCode::POP: {
                return simpleInstruction("POP", offset);
            } break;
            case OpCode::POP_N: {
                return integerInstruction("POP_N", chunk, offset);
            } break;
            case OpCode::POP_N_16: {
                return integer16Instruction("POP_N_16", chunk, offset);
            } break;
            case OpCode::JMP: {
                return integer16Instruction("JMP", chunk, offset);
            } break;
            case OpCode::JMP_IF_FALSE: {
                return integer16Instruction("JMP_IF_FALSE", chunk, offset);
            } break;
            case OpCode::LOOP: {
                return integer16Instruction("LOOP", chunk, offset);
            } break;
            case OpCode::CALL: {
                return integerInstruction("CALL", chunk, offset);
            } break;
            case OpCode::MAKE_CLOSURE: {
                const auto constant = chunk.code[offset + 1];
                printConstantInstruction("MAKE_CLOSURE", chunk, constant);
                return closureUpvalues(chunk, offset + 2);
            } break;
            case OpCode::MAKE_CLOSURE_16: {
                const auto a = chunk.code[offset + 1];
                const auto b = chunk.code[offset + 2];
                const auto constant = parseTwoByteInteger(a, b);

                printConstantInstruction("MAKE_CLOSURE_16", chunk, constant);
                return closureUpvalues(chunk, offset + 3);
            } break;
            case OpCode::READ_UPVALUE: {
                return integerInstruction("READ_UPVALUE", chunk, offset);
            } break;
            case OpCode::SET_UPVALUE: {
                return integerInstruction("SET_UPVALUE", chunk, offset);
            } break;
            case OpCode::CLOSE_UPVALUE: {
                return simpleInstruction("CLOSE_UPVALUE", offset);
            } break;
            case OpCode::MAKE_CLASS: {
                return constantInstruction("MAKE_CLASS", chunk, offset);
            } break;
            case OpCode::MAKE_CLASS_16: {
                return constant16Instruction("MAKE_CLASS_16", chunk, offset);
            } break;
            case OpCode::SET_PROPERTY: {
                return constantInstruction("SET_PROPERTY", chunk, offset);
            } break;
            case OpCode::SET_PROPERTY_16: {
                return constant16Instruction("SET_PROPERTY_16", chunk, offset);
            } break;
            case OpCode::GET_PROPERTY: {
                return constantInstruction("GET_PROPERTY", chunk, offset);
            } break;
            case OpCode::GET_PROPERTY_16: {
                return constant16Instruction("GET_PROPERTY_16", chunk, offset);
            } break;
            case OpCode::METHOD: {
                return constantInstruction("METHOD", chunk, offset);
            } break;
            case OpCode::METHOD_16: {
                return constant16Instruction("METHOD_16", chunk, offset);
            } break;
            case OpCode::INHERIT: {
                return simpleInstruction("INHERIT", offset);
            } break;
            case OpCode::GET_SUPER: {
                return constantInstruction("GET_SUPER", chunk, offset);
            } break;
            case OpCode::GET_SUPER_16: {
                return constant16Instruction("GET_SUPER_16", chunk, offset);
            } break;
            case OpCode::INVOKE: {
                return invokeInstruction("INVOKE", chunk, offset);
            } break;
            case OpCode::INVOKE_16: {
                return invoke16Instruction("INVOKE_16", chunk, offset);
            } break;
            case OpCode::SUPER_INVOKE: {
                return invokeInstruction("SUPER_INVOKE", chunk, offset);
            } break;
            case OpCode::SUPER_INVOKE_16: {
                return invoke16Instruction("SUPER_INVOKE_16", chunk, offset);
            } break;
            default: {
                println("Unknown opcode '{}'",
                        static_cast<std::uint8_t>(opCode));
                return offset + 1;
            } break;
        }
    }

    std::size_t Disassembler::simpleInstruction(const char* name,
                                                std::size_t offset) const {
        println("{}", name);
        return offset + 1;
    }

    std::size_t Disassembler::constantInstruction(const char* name,
                                                  const Chunk& chunk,
                                                  std::size_t offset) const {
        auto constant = chunk.code[offset + 1];
        printConstantInstruction(name, chunk, constant);

        return offset + 2;
    }

    std::size_t Disassembler::constant16Instruction(const char* name,
                                                    const Chunk& chunk,
                                                    std::size_t offset) const {
        auto a = chunk.code[offset + 1];
        auto b = chunk.code[offset + 2];
        auto constant = parseTwoByteInteger(a, b);
        printConstantInstruction(name, chunk, constant);

        return offset + 3;
    }

    std::size_t Disassembler::integerInstruction(const char* name,
                                                 const Chunk& chunk,
                                                 std::size_t offset) const {
        auto operand = chunk.code[offset + 1];
        printIntegerInstruction(name, operand);

        return offset + 2;
    }

    std::size_t Disassembler::integer16Instruction(const char* name,
                                                   const Chunk& chunk,
                                                   std::size_t offset) const {
        auto a = chunk.code[offset + 1];
        auto b = chunk.code[offset + 2];
        auto operand = parseTwoByteInteger(a, b);
        printIntegerInstruction(name, operand);

        return offset + 3;
    }

    void Disassembler::printConstantInstruction(const char* name,
                                                const Chunk& chunk,
                                                std::size_t constIndex) const {
        const Value& v = chunk.constants[constIndex];
        println("{:<16} {:>5} '{}'", name, constIndex, v);
    }

    void Disassembler::printIntegerInstruction(const char* name,
                                               std::size_t operand) const {
        println("{:<16} {:>5}", name, operand);
    }

    void Disassembler::printInvokeInstruction(const char* name,
                                              std::size_t constant,
                                              std::size_t argc,
                                              const Chunk& chunk) const {
        println("{:<16} {:<5} {:>5} '{}'",
                name,
                constant,
                argc,
                chunk.constants[constant]);
    }

    std::size_t Disassembler::invokeInstruction(const char* name,
                                                const Chunk& chunk,
                                                std::size_t offset) const {
        const auto constant = chunk.code[offset + 1];
        const auto count = chunk.code[offset + 2];
        printInvokeInstruction(name, constant, count, chunk);
        return offset + 3;
    }

    std::size_t Disassembler::invoke16Instruction(const char* name,
                                                  const Chunk& chunk,
                                                  std::size_t offset) const {
        const auto a = chunk.code[offset + 1];
        const auto b = chunk.code[offset + 2];
        const auto constant = parseTwoByteInteger(a, b);
        const auto count = chunk.code[offset + 3];
        printInvokeInstruction(name, constant, count, chunk);
        return offset + 4;
    }

    std::size_t Disassembler::closureUpvalues(const Chunk& chunk,
                                              std::size_t offset) const {
        const std::uint8_t count = chunk.code[offset++];
        println("{:04}     | argc = {:>4}", offset - 1, count);

        for (std::uint8_t i = 0; i < count; ++i) {
            const bool local = chunk.code[offset] == 1;
            if (local) {
                const auto a = chunk.code[offset + 1];
                const auto b = chunk.code[offset + 2];
                const auto idx = parseTwoByteInteger(a, b);
                println("{:04}     | local {:>4}", offset, idx);

                offset += 3;
            } else {
                const auto idx = chunk.code[offset + 1];
                println("{:04}     | upvalue {:>4}", offset, idx);

                offset += 2;
            }
        }

        return offset;
    }
} // namespace cpplox