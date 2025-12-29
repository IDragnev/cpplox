add_executable(compiler_test
  main.cpp
  Scanner.cpp
)
target_link_libraries(compiler_test compiler doctest)
target_compile_options(compiler_test
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)