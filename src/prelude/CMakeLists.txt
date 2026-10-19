set(PRELUDE_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/prelude)
set(PRELUDE_LOX ${PROJECT_SOURCE_DIR}/prelude.lox)
set(PRELUDE_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/Prelude.cpp.in)
set(PRELUDE_EMBED_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/EmbedPrelude.cmake)
set(PRELUDE_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/Prelude.cpp)

add_custom_command(
  OUTPUT ${PRELUDE_GENERATED}
  COMMAND ${CMAKE_COMMAND}
          -DINPUT=${PRELUDE_LOX}
          -DTEMPLATE=${PRELUDE_TEMPLATE}
          -DOUTPUT=${PRELUDE_GENERATED}
          -P ${PRELUDE_EMBED_SCRIPT}
  DEPENDS ${PRELUDE_LOX} ${PRELUDE_TEMPLATE} ${PRELUDE_EMBED_SCRIPT}
  COMMENT "Embedding prelude.lox"
  VERBATIM
)

add_library(
  prelude
  STATIC
  ${PRELUDE_HEADERS_DIR}/Prelude.hpp
  ${PRELUDE_GENERATED}
)
target_include_directories(prelude PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(prelude PUBLIC cxx_std_20)
target_compile_options(prelude
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)
