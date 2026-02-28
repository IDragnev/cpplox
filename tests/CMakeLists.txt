add_executable(core_test
  core/main.cpp
  core/String.cpp
  core/Value.cpp
  core/Vector.cpp
  core/Algorithm.cpp
  core/ValueStack.cpp
  core/ValueMap.cpp
)
target_link_libraries(core_test corelib doctest)
target_compile_options(core_test
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

add_executable(compiler_test
  compiler/main.cpp
  compiler/Scanner.cpp
)
target_link_libraries(compiler_test compiler doctest)
target_compile_options(compiler_test
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)

add_executable(bytecode_test
  bytecode/main.cpp
  bytecode/Bytecode.cpp
)
target_link_libraries(bytecode_test bytecode doctest)
target_compile_options(bytecode_test
 PRIVATE ${CPPLOX_TARGET_WARNING_FLAGS}
)