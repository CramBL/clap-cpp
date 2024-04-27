if(WIN32)
  # Because of the `-t` option when including runtime dlls
  cmake_minimum_required(VERSION 3.26) # LLVM 16.0.0 requires >= 3.20.0
else()
  cmake_minimum_required(VERSION 3.21)
endif()

project(clap_cppTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----
if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)

  enable_testing()

  find_package(clap_cpp CONFIG REQUIRED) # for project_options, ...

  if(NOT TARGET clap_cpp_options)
    message(FATAL_ERROR "Required config package not found!")
    return()
  endif()
endif()

# ---- Dependencies ----
include(../cmake/SystemLink.cmake)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
find_package(Catch2 3 REQUIRED)
include(Catch)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE
                                     clap_cpp::clap_cpp_warnings
                                     clap_cpp::clap_cpp_options
                                     clap_cpp)
target_link_system_library(tests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(
  tests
  TEST_PREFIX
  "tests."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "tests."
  OUTPUT_SUFFIX
  .xml)

