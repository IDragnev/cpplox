add_executable(cpplox_exe main.cpp)
set_target_properties(cpplox_exe PROPERTIES OUTPUT_NAME cpplox)
target_compile_features(cpplox_exe PRIVATE cxx_std_20)
target_link_libraries(cpplox_exe
  compiler
  vm
  prelude
  loglib
)

target_compile_options(cpplox_exe
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

target_compile_definitions(cpplox_exe
  PRIVATE CPPLOX_VERSION="${PROJECT_VERSION}"
)

install(TARGETS cpplox_exe RUNTIME DESTINATION .)