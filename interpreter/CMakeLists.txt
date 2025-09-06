add_executable(cpplox_exe main.cpp)
set_target_properties(cpplox_exe PROPERTIES OUTPUT_NAME cpplox)
target_compile_features(cpplox_exe PRIVATE cxx_std_20)

target_compile_options(cpplox_exe
  PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)