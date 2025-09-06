cmake_minimum_required(VERSION 3.14)

project(cpplox
  LANGUAGES CXX
)

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

# CPPLOX targets
add_subdirectory(interpreter)