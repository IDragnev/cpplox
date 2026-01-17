set(COMPILER_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/compiler)

set(CPPLOX_COMPILER_SOURCE
  ${COMPILER_HEADERS_DIR}/Token.hpp
  ${COMPILER_HEADERS_DIR}/Scanner.hpp
  ${COMPILER_HEADERS_DIR}/Compiler.hpp

  Scanner.cpp
  Compiler.cpp
)

add_library(
  compiler
  STATIC
  ${CPPLOX_COMPILER_SOURCE}
)
target_link_libraries(compiler
  PUBLIC corelib
  PUBLIC bytecode
  PUBLIC runtime
  PUBLIC diagnostics
)
target_include_directories(compiler PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(compiler PUBLIC cxx_std_20)
target_compile_options(compiler
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)