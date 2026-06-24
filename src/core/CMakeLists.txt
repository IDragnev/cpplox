set(CORE_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/core)

set(CPPLOX_CORE_SOURCE
  ${CORE_HEADERS_DIR}/String.hpp
  ${CORE_HEADERS_DIR}/StringFormatter.hpp
  ${CORE_HEADERS_DIR}/Value.hpp
  ${CORE_HEADERS_DIR}/ValueFormatter.hpp
  ${CORE_HEADERS_DIR}/Format.hpp
  ${CORE_HEADERS_DIR}/Vector.hpp
  ${CORE_HEADERS_DIR}/VectorImpl.hpp
  ${CORE_HEADERS_DIR}/ValueStack.hpp
  ${CORE_HEADERS_DIR}/ValueMap.hpp
  ${CORE_HEADERS_DIR}/Algorithm.hpp

  String.cpp
  Value.cpp
  ValueStack.cpp
  ValueMap.cpp
)

add_library(
  corelib 
  STATIC
  ${CPPLOX_CORE_SOURCE}
)
target_link_libraries(corelib PUBLIC fmt::fmt)
target_include_directories(corelib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(corelib PUBLIC cxx_std_20)
target_compile_options(corelib
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)