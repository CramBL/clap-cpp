find_package(fmt)
add_executable(example main.cpp)

target_link_libraries(
  example
  PRIVATE clap_cpp::clap_cpp_options
          clap_cpp::clap_cpp_warnings
          )

target_link_system_libraries(
  example
  PRIVATE
          fmt::fmt
          spdlog::spdlog
          clap_cpp
          )

target_include_directories(example PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
include(../../cmake/Utilities.cmake)
copy_runtime_dlls(example)
