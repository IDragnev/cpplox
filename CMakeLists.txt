cmake_minimum_required(VERSION 3.14)

project(cpplox
  LANGUAGES CXX
)

include(FetchContent)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
  add_definitions(-DNDEBUG)
endif()

option(CPPLOX_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(CPPLOX_DEBUG_TRACE_EXECUTION "Trace the interpreter execution" OFF)
option(CPPLOX_DEBUG_LOG_GC "Trace the garbage collector" OFF)
option(CPPLOX_DEBUG_STRESS_GC "Stress test the garbage collector" OFF)

if(CPPLOX_DEBUG_TRACE_EXECUTION)
  add_compile_definitions(CPPLOX_DEBUG_TRACE_EXECUTION)
endif()
if(CPPLOX_DEBUG_LOG_GC)
  add_compile_definitions(CPPLOX_DEBUG_LOG_GC)
endif()
if(CPPLOX_DEBUG_STRESS_GC)
  add_compile_definitions(CPPLOX_DEBUG_STRESS_GC)
endif()

if(MSVC)
  set(CPPLOX_TARGET_WARNING_FLAGS
    "/W4"
    $<$<BOOL:${CPPLOX_TREAT_WARNINGS_AS_ERRORS}>:/WX>
    "/w15038" "/w14800" "/w14242" "/w14254" "/w14263" "/w14264" "/w14265" "/w14266"
    "/w14287" "/w14296" "/w14388" "/w14471" "/w14619" "/w14800" "/w14946"
  )
else()
  set(CPPLOX_TARGET_WARNING_FLAGS "-Wall" "-Wextra" $<$<BOOL:${CPPLOX_TREAT_WARNINGS_AS_ERRORS}>:-Werror> "-pedantic")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# DEPENDENCIES
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest.git
  GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(doctest)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(fmt)

# CPPLOX targets
add_subdirectory(include/cpplox/log)
add_subdirectory(include/cpplox/diagnostics)
add_subdirectory(src/core)
add_subdirectory(src/bytecode)
add_subdirectory(src/runtime)
add_subdirectory(src/vm)
add_subdirectory(src/compiler)
add_subdirectory(app)
add_subdirectory(tests)