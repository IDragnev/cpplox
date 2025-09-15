set(DEBUG_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/debug)

set(CPPLOX_DEBUG_SOURCE
   ${DEBUG_HEADERS_DIR}/Disassembler.hpp

  Disassembler.cpp
)

add_library(
  debuglib
  STATIC
  ${CPPLOX_DEBUG_SOURCE}
)
target_link_libraries(debuglib
  PRIVATE corelib
)
target_include_directories(debuglib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(debuglib PUBLIC cxx_std_20)
target_compile_options(debuglib
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)