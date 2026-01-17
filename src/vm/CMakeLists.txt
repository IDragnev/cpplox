set(VM_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/cpplox/vm)

set(CPPLOX_VM_SOURCE
  ${VM_HEADERS_DIR}/VM.hpp

  VM.cpp
)

add_library(
  vm
  STATIC
  ${CPPLOX_VM_SOURCE}
)
target_link_libraries(vm
  PUBLIC corelib
  PUBLIC bytecode
  PUBLIC runtime
  PRIVATE loglib
)
target_include_directories(vm PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(vm PUBLIC cxx_std_20)
target_compile_options(vm
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

