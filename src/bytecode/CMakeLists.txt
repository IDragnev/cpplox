set(BYTECODE_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/bytecode)

set(CPPLOX_BYTECODE_SOURCE
  ${BYTECODE_HEADERS_DIR}/OpCode.hpp
  ${BYTECODE_HEADERS_DIR}/Chunk.hpp
  ${BYTECODE_HEADERS_DIR}/Bytecode.hpp
  ${BYTECODE_HEADERS_DIR}/Disassembler.hpp

  Bytecode.cpp
  Disassembler.cpp
)

add_library(
  bytecode
  STATIC
  ${CPPLOX_BYTECODE_SOURCE}
)
target_link_libraries(bytecode
  PUBLIC corelib
)
target_include_directories(bytecode PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(bytecode PUBLIC cxx_std_20)
target_compile_options(bytecode
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

