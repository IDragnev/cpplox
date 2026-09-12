if(WIN32)
  set(CPPLOX_PACKAGE_OS windows)
elseif(APPLE)
  set(CPPLOX_PACKAGE_OS macos)
else()
  set(CPPLOX_PACKAGE_OS linux)
endif()

if(APPLE AND CMAKE_OSX_ARCHITECTURES)
  set(CPPLOX_PACKAGE_ARCH "${CMAKE_OSX_ARCHITECTURES}")
elseif(CMAKE_GENERATOR_PLATFORM)
  set(CPPLOX_PACKAGE_ARCH "${CMAKE_GENERATOR_PLATFORM}")
else()
  set(CPPLOX_PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif()

string(TOLOWER "${CPPLOX_PACKAGE_ARCH}" CPPLOX_PACKAGE_ARCH)
if(CPPLOX_PACKAGE_ARCH MATCHES "^(x64|x86_64|amd64)$")
  set(CPPLOX_PACKAGE_ARCH x64)
elseif(CPPLOX_PACKAGE_ARCH MATCHES "^(arm64|aarch64)$")
  set(CPPLOX_PACKAGE_ARCH arm64)
endif()

# The documentation link is pinned to this version's tag, so an archive always
# points at the language the binary beside it implements, not at main.
configure_file(
  "${PROJECT_SOURCE_DIR}/packaging/INSTALL.txt.in"
  "${PROJECT_BINARY_DIR}/INSTALL.txt"
  @ONLY
)

install(FILES
  "${PROJECT_SOURCE_DIR}/LICENSE"
  "${PROJECT_BINARY_DIR}/INSTALL.txt"
  DESTINATION .
)

set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}")

set(CPACK_PACKAGE_NAME cpplox)
set(CPACK_PACKAGE_VENDOR "IDragnev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An interpreter for the Lox language")
set(CPACK_PACKAGE_FILE_NAME
  "cpplox-${PROJECT_VERSION}-${CPPLOX_PACKAGE_OS}-${CPPLOX_PACKAGE_ARCH}")
set(CPACK_STRIP_FILES ON)
set(CPACK_VERBATIM_VARIABLES ON)

if(WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()

include(CPack)
