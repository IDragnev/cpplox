#include "cpplox/core/VM.hpp"
#include "cpplox/debug/Disassembler.hpp"

using cpplox::Chunk;
using cpplox::OpCode;
using cpplox::debug::Disassembler;

int main() {
    Chunk chunk;
    
    auto offset = cpplox::addConstant(chunk, 2.2);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::CONSTANT), 123u);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(offset), 123u);

    offset = cpplox::addConstant(chunk, 3.4);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::CONSTANT), 123u);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(offset), 123u);

    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::ADD), 123u);

    offset = cpplox::addConstant(chunk, 5.6);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::CONSTANT), 123u);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(offset), 123u);

    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::DIVIDE), 123u);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::NEGATE), 123u);
    cpplox::addCode(chunk, static_cast<std::uint8_t>(OpCode::RETURN), 123u);

    cpplox::VM vm;
    auto res = vm.interpret(chunk);
    (void)res;

    Disassembler disassembler;
    disassembler.disassembleChunk(chunk, "test chunk");

    return 0;
}