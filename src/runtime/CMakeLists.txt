set(RUNTIME_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/runtime)

set(CPPLOX_RUNTIME_SOURCE
  ${RUNTIME_HEADERS_DIR}/Object.hpp
  ${RUNTIME_HEADERS_DIR}/Function.hpp
  ${RUNTIME_HEADERS_DIR}/Closure.hpp
  ${RUNTIME_HEADERS_DIR}/Upvalue.hpp
  ${RUNTIME_HEADERS_DIR}/GCVisitor.hpp
  ${RUNTIME_HEADERS_DIR}/GC.hpp
  ${RUNTIME_HEADERS_DIR}/Class.hpp

  Function.cpp
  Closure.cpp
  Upvalue.cpp
  Class.cpp
  GC.cpp
)

add_library(
  runtime
  STATIC
  ${CPPLOX_RUNTIME_SOURCE}
)
target_link_libraries(runtime
  PUBLIC loglib
  PUBLIC corelib
  PUBLIC bytecode
)
target_include_directories(runtime PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(runtime PUBLIC cxx_std_20)
target_compile_options(runtime
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

