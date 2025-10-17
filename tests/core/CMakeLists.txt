add_executable(core_test
  main.cpp
  Scanner.cpp
)
target_link_libraries(core_test corelib doctest)
target_compile_options(core_test
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)