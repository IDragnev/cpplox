add_library(diagnostics INTERFACE)
target_link_libraries(diagnostics INTERFACE
  fmt::fmt
)
target_include_directories(diagnostics INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(diagnostics INTERFACE cxx_std_20)
