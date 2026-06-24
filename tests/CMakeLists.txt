FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest.git
  GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(doctest)

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

add_test(NAME core_test COMMAND core_test)
add_test(NAME compiler_test COMMAND compiler_test)
add_test(NAME bytecode_test COMMAND bytecode_test)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(
    NAME e2e_tests
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/e2e/test_runner.py
            --interpreter $<TARGET_FILE:cpplox_exe>
            -v
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(e2e_tests PROPERTIES TIMEOUT 120)