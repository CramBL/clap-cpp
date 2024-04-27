if(WIN32)
  # Because of the `-t` option when including runtime dlls
  cmake_minimum_required(VERSION 3.26) # LLVM 16.0.0 requires >= 3.20.0
else()
  cmake_minimum_required(VERSION 3.21)
endif()

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

project(
   clap_cpp
   VERSION 0.1.0
   DESCRIPTION "Command-line Argument Parser library"
   HOMEPAGE_URL "https://github.com/CramBL/clap-cpp"
   LANGUAGES CXX
)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)

clap_cpp_setup_options()

clap_cpp_global_options()
include(Dependencies.cmake)
clap_cpp_setup_dependencies()

clap_cpp_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(GIT_SHA
    "Unknown"
    CACHE STRING "SHA this build was generated from")
string(
  SUBSTRING "${GIT_SHA}"
            0
            8
            GIT_SHORT_SHA)

target_compile_features(clap_cpp_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})


add_library(clap_cpp::clap_cpp_options ALIAS clap_cpp_options)
add_library(clap_cpp::clap_cpp_warnings ALIAS clap_cpp_warnings)
add_subdirectory(configured_files)

# Define the library target
add_library(clap_cpp INTERFACE)
target_include_directories(clap_cpp INTERFACE
$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
$<INSTALL_INTERFACE:include>
)
include(cmake/SystemLink.cmake)
target_link_system_library(${PROJECT_NAME} INTERFACE fmt::fmt)
target_link_system_library(${PROJECT_NAME} INTERFACE spdlog::spdlog)

# Build an example CLI in src
add_subdirectory(src)

# Don't even look at tests if we're not top level
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

# Adding the tests:
include(CTest)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  get_all_installable_targets(all_targets)
  message("all_targets=${all_targets}")
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT clap_cpp)

if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()

include(cmake/PackageProject.cmake)

# Package the example binary
clap_cpp_package_project(
 TARGETS
 example
 VERSION 0.1.0
 clap_cpp_options
 clap_cpp_warnings
)
