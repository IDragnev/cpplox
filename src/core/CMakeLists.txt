set(CORE_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/core)

set(CPPLOX_CORE_SOURCE
  ${CORE_HEADERS_DIR}/Value.hpp
  ${CORE_HEADERS_DIR}/Chunk.hpp
  ${CORE_HEADERS_DIR}/ValueStack.hpp
  ${CORE_HEADERS_DIR}/VM.hpp
  ${CORE_HEADERS_DIR}/Scanner.hpp
  ${CORE_HEADERS_DIR}/Compiler.hpp

  Value.cpp
  ValueStack.cpp
  VM.cpp
  Scanner.cpp
  Compiler.cpp
)

add_library(
  corelib 
  STATIC
  ${CPPLOX_CORE_SOURCE}
)
target_include_directories(corelib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(corelib PUBLIC cxx_std_20)
target_compile_options(corelib
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)