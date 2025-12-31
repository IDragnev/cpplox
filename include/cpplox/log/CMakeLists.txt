add_library(loglib INTERFACE)
target_link_libraries(loglib INTERFACE
    fmt::fmt
)
target_include_directories(loglib INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(loglib INTERFACE cxx_std_20)
