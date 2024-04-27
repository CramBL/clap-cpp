include(cmake/SystemLink.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(clap_cpp_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(clap_cpp_setup_options)
  option(clap_cpp_ENABLE_HARDENING "Enable hardening" ON)
  option(clap_cpp_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    clap_cpp_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to build dependencies"
    ON
    clap_cpp_ENABLE_HARDENING
    OFF)

  clap_cpp_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR clap_cpp_PACKAGING_MAINTAINER_MODE)
    option(clap_cpp_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(clap_cpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(clap_cpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(clap_cpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(clap_cpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(clap_cpp_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(clap_cpp_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(clap_cpp_ENABLE_PCH "Enable precompiled headers" OFF)
    option(clap_cpp_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(clap_cpp_ENABLE_IPO "Enable IPO/LTO" ON)
    option(clap_cpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(clap_cpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(clap_cpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(clap_cpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(clap_cpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(clap_cpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(clap_cpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(clap_cpp_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(clap_cpp_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(clap_cpp_ENABLE_PCH "Enable precompiled headers" OFF)
    option(clap_cpp_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      clap_cpp_ENABLE_IPO
      clap_cpp_WARNINGS_AS_ERRORS
      clap_cpp_ENABLE_USER_LINKER
      clap_cpp_ENABLE_SANITIZER_ADDRESS
      clap_cpp_ENABLE_SANITIZER_LEAK
      clap_cpp_ENABLE_SANITIZER_UNDEFINED
      clap_cpp_ENABLE_SANITIZER_THREAD
      clap_cpp_ENABLE_SANITIZER_MEMORY
      clap_cpp_ENABLE_UNITY_BUILD
      clap_cpp_ENABLE_CLANG_TIDY
      clap_cpp_ENABLE_CPPCHECK
      clap_cpp_ENABLE_COVERAGE
      clap_cpp_ENABLE_PCH
      clap_cpp_ENABLE_CACHE)
  endif()
endmacro()

macro(clap_cpp_global_options)
  if(clap_cpp_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    clap_cpp_enable_ipo()
  endif()

  clap_cpp_supports_sanitizers()

  if(clap_cpp_ENABLE_HARDENING AND clap_cpp_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN
       OR clap_cpp_ENABLE_SANITIZER_UNDEFINED
       OR clap_cpp_ENABLE_SANITIZER_ADDRESS
       OR clap_cpp_ENABLE_SANITIZER_THREAD
       OR clap_cpp_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("HARDENING=${clap_cpp_ENABLE_HARDENING} UBSAN_MIN_RT=${ENABLE_UBSAN_MINIMAL_RUNTIME} UB_SAN=${clap_cpp_ENABLE_SANITIZER_UNDEFINED}")
    clap_cpp_enable_hardening(clap_cpp_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(clap_cpp_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(clap_cpp_warnings INTERFACE)
  add_library(clap_cpp_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  clap_cpp_set_project_warnings(
    clap_cpp_warnings
    ${clap_cpp_WARNINGS_AS_ERRORS}
    ""
    ""
    "")

  if(clap_cpp_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(clap_cpp_options)
  endif()

  include(cmake/Sanitizers.cmake)
  clap_cpp_enable_sanitizers(
    clap_cpp_options
    ${clap_cpp_ENABLE_SANITIZER_ADDRESS}
    ${clap_cpp_ENABLE_SANITIZER_LEAK}
    ${clap_cpp_ENABLE_SANITIZER_UNDEFINED}
    ${clap_cpp_ENABLE_SANITIZER_THREAD}
    ${clap_cpp_ENABLE_SANITIZER_MEMORY})

  set_target_properties(clap_cpp_options PROPERTIES UNITY_BUILD ${clap_cpp_ENABLE_UNITY_BUILD})

  if(clap_cpp_ENABLE_PCH)
    target_precompile_headers(
      clap_cpp_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(clap_cpp_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    clap_cpp_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(clap_cpp_ENABLE_CLANG_TIDY)
    clap_cpp_enable_clang_tidy(clap_cpp_options ${clap_cpp_WARNINGS_AS_ERRORS})
  endif()

  if(clap_cpp_ENABLE_CPPCHECK)
    clap_cpp_enable_cppcheck(${clap_cpp_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(clap_cpp_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    clap_cpp_enable_coverage(clap_cpp_options)
  endif()

  if(clap_cpp_ENABLE_HARDENING AND NOT clap_cpp_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN
       OR clap_cpp_ENABLE_SANITIZER_UNDEFINED
       OR clap_cpp_ENABLE_SANITIZER_ADDRESS
       OR clap_cpp_ENABLE_SANITIZER_THREAD
       OR clap_cpp_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    clap_cpp_enable_hardening(clap_cpp_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
