set(RUNTIME_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/runtime)

set(CPPLOX_RUNTIME_SOURCE
  ${RUNTIME_HEADERS_DIR}/Object.hpp
  ${RUNTIME_HEADERS_DIR}/Function.hpp
  ${RUNTIME_HEADERS_DIR}/Closure.hpp
  ${RUNTIME_HEADERS_DIR}/Upvalue.hpp
  ${RUNTIME_HEADERS_DIR}/GCVisitor.hpp
  ${RUNTIME_HEADERS_DIR}/Memory.hpp
  ${RUNTIME_HEADERS_DIR}/Heap.hpp
  ${RUNTIME_HEADERS_DIR}/Class.hpp
  ${RUNTIME_HEADERS_DIR}/Instance.hpp
  ${RUNTIME_HEADERS_DIR}/BoundMethod.hpp
  ${RUNTIME_HEADERS_DIR}/NativeFunction.hpp
  ${RUNTIME_HEADERS_DIR}/NativeContext.hpp
  ${RUNTIME_HEADERS_DIR}/List.hpp
  ${RUNTIME_HEADERS_DIR}/Natives.hpp
  ${RUNTIME_HEADERS_DIR}/ObjectFormatter.hpp
  ${RUNTIME_HEADERS_DIR}/Display.hpp
  ${RUNTIME_HEADERS_DIR}/Reflect.hpp

  Function.cpp
  Closure.cpp
  Upvalue.cpp
  Class.cpp
  Instance.cpp
  BoundMethod.cpp
  NativeFunction.cpp
  NativeContext.cpp
  List.cpp
  Natives.cpp
  Display.cpp
  Reflect.cpp
  Memory.cpp
  Heap.cpp
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

