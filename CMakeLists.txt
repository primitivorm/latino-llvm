cmake_minimum_required(VERSION 3.14.0)

if ("${CMAKE_VERSION}" VERSION_LESS "3.13.4")
  message(WARNING
    "Your CMake version is ${CMAKE_VERSION}. Starting with LLVM 12.0.0, the "
    "minimum version of CMake required to build LLVM will become 3.13.4, and "
    "using an older CMake will become an error. Please upgrade your CMake to "
    "at least 3.13.4 now to avoid issues in the future!")
endif()

# if(POLICY CMP0068)
#   cmake_policy(SET CMP0068 NEW)
#   set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
# endif()

# if(POLICY CMP0075)
#   cmake_policy(SET CMP0075 NEW)
# endif()

# if(POLICY CMP0077)
#   cmake_policy(SET CMP0077 NEW)
# endif()

project(latino C CXX)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_CXX_EXTENSIONS: ${CMAKE_CXX_EXTENSIONS}")

set(LLVM_TOOLS_BINARY_DIR ${TOOLS_BINARY_DIR} CACHE PATH "Path to llvm/bin")
set(LLVM_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to llvm/lib")
set(LLVM_MAIN_INCLUDE_DIR ${INCLUDE_DIR} CACHE PATH "Path to llvm/include")
set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT} CACHE PATH "Path to LLVM build tree")
set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE) # Default to a Release build
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build [Release Debug]" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
  # set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(LATINO_ENABLE_STATIC_ANALYZER ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
if(WIN32)
  set(LLVM_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/${CMAKE_BUILD_TYPE}/bin")
  message(STATUS "LLVM_BINARY_DIR: ${LLVM_BINARY_DIR}")
endif()

if(NOT LLVM_TOOLS_BINARY_DIR)
  set(LLVM_TOOLS_BINARY_DIR ${LLVM_BINARY_DIR})
endif()

message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set(LLVM_MAIN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/llvm")
message(STATUS "LLVM_MAIN_SRC_DIR: ${LLVM_MAIN_SRC_DIR}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/" "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/llvm/cmake/modules/" "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/latino/cmake/modules/")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

set(CONFIG_OUTPUT)
find_program(LLVM_CONFIG "llvm-config" PATHS "${LLVM_BINARY_DIR}")
if(LLVM_CONFIG)
  message(STATUS "Found LLVM_CONFIG as ${LLVM_CONFIG}")
  set(CONFIG_COMMAND ${LLVM_CONFIG}
    "--assertion-mode"
    "--bindir"
    "--libdir"
    "--includedir"
    "--prefix"
    "--src-root"
    "--cmakedir")
  execute_process(COMMAND ${CONFIG_COMMAND}
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE CONFIG_OUTPUT)
  if(NOT HAD_ERROR)
    string(REGEX REPLACE
      "[ \t]*[\r\n]+[ \t]*" ";"
      CONFIG_OUTPUT ${CONFIG_OUTPUT})
  else()
    string(REPLACE ";" " " CONFIG_COMMAND_STR "${CONFIG_COMMAND}")
    message(STATUS "${CONFIG_COMMAND_STR}")
    message(FATAL_ERROR "llvm-config not found -- ${LLVM_CONFIG}")
  endif()

  list(GET CONFIG_OUTPUT 0 ENABLE_ASSERTIONS)
  list(GET CONFIG_OUTPUT 1 TOOLS_BINARY_DIR)
  list(GET CONFIG_OUTPUT 2 LIBRARY_DIR)
  list(GET CONFIG_OUTPUT 3 INCLUDE_DIR)
  list(GET CONFIG_OUTPUT 4 LLVM_OBJ_ROOT)
  list(GET CONFIG_OUTPUT 5 MAIN_SRC_DIR)
  list(GET CONFIG_OUTPUT 6 LLVM_CONFIG_CMAKE_PATH)

  if(NOT MSVC_IDE)
    set(LLVM_ENABLE_ASSERTIONS ${ENABLE_ASSERTIONS}
      CACHE BOOL "Enable assertions")
    mark_as_advanced(LLVM_ENABLE_ASSERTIONS)
  endif()

  file(TO_CMAKE_PATH ${LLVM_CONFIG_CMAKE_PATH} LLVM_CMAKE_PATH)

  find_program(LLVM_TABLEGEN_EXE "llvm-tblgen" ${LLVM_TOOLS_BINARY_DIR}
    NO_DEFAULT_PATH)
  message(STATUS "LLVM_TOOLS_BINARY_DIR: ${LLVM_TOOLS_BINARY_DIR}")
  
  # find_program(LATINO_TABLEGEN_EXE "latino-tblgen" ${LLVM_TOOLS_BINARY_DIR}
  # NO_DEFAULT_PATH)
  # message(STATUS "LATINO_TABLEGEN_EXE: ${LATINO_TABLEGEN_EXE}")
  # set(LATINO_TABLEGEN_TARGET ${LATINO_TABLEGEN_EXE})

  option(LLVM_INSTALL_TOOLCHAIN_ONLY
  "Only include toolchain files in the 'install' target." OFF)

  if(WIN32)
    set(LLVMCONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/cmake/modules/CMakeFiles/LLVMConfig.cmake")
  endif()
  if(EXISTS ${LLVMCONFIG_FILE})
    include(${LLVMCONFIG_FILE})
  else()
    message(FATAL_ERROR "Not found: ${LLVMCONFIG_FILE}")
  endif()
  message(STATUS "LLVMCONFIG_FILE: ${LLVMCONFIG_FILE}")
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
  if(WIN32 OR CYGWIN)
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_RUNTIME_OUTPUT_INTDIR})
  else()
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_LIBRARY_OUTPUT_INTDIR})
  endif()
  message(STATUS "LLVM_RUNTIME_OUTPUT_INTDIR: ${LLVM_RUNTIME_OUTPUT_INTDIR}")
  message(STATUS "LLVM_LIBRARY_OUTPUT_INTDIR: ${LLVM_LIBRARY_OUTPUT_INTDIR}")
  message(STATUS "LLVM_SHLIB_OUTPUT_INTDIR: ${LLVM_SHLIB_OUTPUT_INTDIR}")
endif()

option(LATINO_INCLUDE_TESTS "Generate build targets for the Latino unit tests." OFF)
option(LATINO_BUILD_TOOLS
  "Build the Latino tools. If OFF, just generate build targets." ON)

# set(LLVM_TARGETS_TO_BUILD "all")
set(LLVM_TARGETS_TO_BUILD host CACHE STRING "")
set(LLVM_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/${CMAKE_BUILD_TYPE}/lib/)

if(NOT LLVM_TARGETS_TO_BUILD)
  set(LLVM_TARGETS_TO_BUILD "host")
endif()
message(STATUS "LLVM_TARGETS_TO_BUILD: ${LLVM_TARGETS_TO_BUILD}")

include(CMakeParseArguments)
include(AddLLVM)
include(TableGen)
# include(AddClang)
include(AddLatino)

set(PACKAGE_VERSION "${LLVM_PACKAGE_VERSION}")
  set(BUG_REPORT_URL "${LLVM_PACKAGE_BUGREPORT}" CACHE STRING
    "Default URL where bug reports are to be submitted.")

  if (NOT DEFINED LLVM_INCLUDE_TESTS)
    set(LLVM_INCLUDE_TESTS ON)
  endif()

  include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_MAIN_INCLUDE_DIR}")
  link_directories("${LLVM_LIBRARY_DIR}")

  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib${LLVM_LIBDIR_SUFFIX} )
  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib${LLVM_LIBDIR_SUFFIX} )

  if(LLVM_INCLUDE_TESTS)
    if(CMAKE_VERSION VERSION_LESS 3.12)
      include(FindPythonInterp)
      if(NOT PYTHONINTERP_FOUND)
        message(FATAL_ERROR
  "Unable to find Python interpreter, required for builds and testing.

  Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
      endif()

      if( ${PYTHON_VERSION_STRING} VERSION_LESS 2.7 )
        message(FATAL_ERROR "Python 2.7 or newer is required")
      endif()

      add_executable(Python3::Interpreter IMPORTED)
      set_target_properties(Python3::Interpreter PROPERTIES
        IMPORTED_LOCATION ${PYTHON_EXECUTABLE})
      set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
    else()
      find_package(Python3 COMPONENTS Interpreter)
      if(NOT Python3_Interpreter_FOUND)
        message(WARNING "Python3 not found, using python2 as a fallback")
        find_package(Python2 COMPONENTS Interpreter REQUIRED)
        if(Python2_VERSION VERSION_LESS 2.7)
          message(SEND_ERROR "Python 2.7 or newer is required")
        endif()

        # Treat python2 as python3
        add_executable(Python3::Interpreter IMPORTED)
        set_target_properties(Python3::Interpreter PROPERTIES
          IMPORTED_LOCATION ${Python2_EXECUTABLE})
        set(Python3_EXECUTABLE ${Python2_EXECUTABLE})
      endif()
    endif()

    # Check prebuilt llvm/utils.
    if(EXISTS ${LLVM_TOOLS_BINARY_DIR}/FileCheck${CMAKE_EXECUTABLE_SUFFIX}
        AND EXISTS ${LLVM_TOOLS_BINARY_DIR}/count${CMAKE_EXECUTABLE_SUFFIX}
        AND EXISTS ${LLVM_TOOLS_BINARY_DIR}/not${CMAKE_EXECUTABLE_SUFFIX})
      set(LLVM_UTILS_PROVIDED ON)
    endif()

    if(EXISTS ${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py)
      # Note: path not really used, except for checking if lit was found
      set(LLVM_LIT ${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py)
      if(EXISTS ${LLVM_MAIN_SRC_DIR}/utils/llvm-lit)
        add_subdirectory(${LLVM_MAIN_SRC_DIR}/utils/llvm-lit utils/llvm-lit)
      endif()
      if(NOT LLVM_UTILS_PROVIDED)
        add_subdirectory(${LLVM_MAIN_SRC_DIR}/utils/FileCheck utils/FileCheck)
        add_subdirectory(${LLVM_MAIN_SRC_DIR}/utils/count utils/count)
        add_subdirectory(${LLVM_MAIN_SRC_DIR}/utils/not utils/not)
        set(LLVM_UTILS_PROVIDED ON)
        set(LATINO_TEST_DEPS FileCheck count not)
      endif()
      set(UNITTEST_DIR ${LLVM_MAIN_SRC_DIR}/utils/unittest)
      if(EXISTS ${UNITTEST_DIR}/googletest/include/gtest/gtest.h
          AND NOT EXISTS ${LLVM_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
          AND EXISTS ${UNITTEST_DIR}/CMakeLists.txt)
        add_subdirectory(${UNITTEST_DIR} utils/unittest)
      endif()
    else()
      # Seek installed Lit.
      find_program(LLVM_LIT
                   NAMES llvm-lit lit.py lit
                   PATHS "${LLVM_MAIN_SRC_DIR}/utils/lit"
                   DOC "Path to lit.py")
    endif()

    if(LLVM_LIT)
      # Define the default arguments to use with 'lit', and an option for the user
      # to override.
      set(LIT_ARGS_DEFAULT "-sv")
      if (MSVC OR XCODE)
        set(LIT_ARGS_DEFAULT "${LIT_ARGS_DEFAULT} --no-progress-bar")
      endif()
      set(LLVM_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")

      # On Win32 hosts, provide an option to specify the path to the GnuWin32 tools.
      if( WIN32 AND NOT CYGWIN )
        set(LLVM_LIT_TOOLS_DIR "" CACHE PATH "Path to GnuWin32 tools")
      endif()
    else()
      set(LLVM_INCLUDE_TESTS OFF)
    endif()
  endif()

  set( LATINO_BUILT_STANDALONE 1 )
  set(BACKEND_PACKAGE_STRING "LLVM ${LLVM_PACKAGE_VERSION}")
else()
  set(BACKEND_PACKAGE_STRING "${PACKAGE_STRING}")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

message(STATUS "LLVM_INSTALL_TOOLCHAIN_ONLY: ${LLVM_INSTALL_TOOLCHAIN_ONLY}")
if(NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
  install(DIRECTORY include/latino
    DESTINATION include
    COMPONENT clang-headers
    FILES_MATCHING
    PATTERN "*.def"
    PATTERN "*.h"
    PATTERN "config.h" EXCLUDE
    PATTERN ".svn" EXCLUDE
  )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/latino
    DESTINATION include
    COMPONENT latino-headers
    FILES_MATCHING
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "*.inc"
    PATTERN "*.h"
    )

  # Installing the headers needs to depend on generating any public
  # tablegen'd headers.
  add_custom_target(latino-headers DEPENDS latino-tablegen-targets)
  set_target_properties(latino-headers PROPERTIES FOLDER "Misc")
  if(NOT LLVM_ENABLE_IDE)
    add_llvm_install_targets(install-latino-headers
    DEPENDS latino-headers
    COMPONENT latino-headers)
  endif()

  # add_custom_target(bash-autocomplete DEPENDS utils/bash-autocomplete.sh)
  # install(PROGRAMS utils/bash-autocomplete.sh
  #         DESTINATION share/clang
  #         COMPONENT bash-autocomplete)
  # if(NOT LLVM_ENABLE_IDE)
  #   add_llvm_install_targets(install-bash-autocomplete
  #                            DEPENDS bash-autocomplete
  #                            COMPONENT bash-autocomplete)
  # endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LATINO_LIBS}")

set(LATINO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LATINO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(BACKEND_PACKAGE_STRING "LLVM ${LLVM_PACKAGE_VERSION}")

#Latino version
set(LATINO_VERSION_MAJOR 11)
set(LATINO_VERSION_MINOR 0)
set(LATINO_VERSION_PATCHLEVEL 0)
set(LATINO_VERSION "${LATINO_VERSION_MAJOR}.${LATINO_VERSION_MINOR}.${LATINO_VERSION_PATCHLEVEL}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/latino/Basic/Version.inc.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/latino/Basic/Version.inc)

configure_file(
  ${LATINO_SOURCE_DIR}/include/latino/Config/config.h.cmake
  ${LATINO_BINARY_DIR}/include/latino/Config/config.h)

if(LLVM_BUILD_MAIN_SRC_DIR)
  include_directories(${LLVM_BUILD_MAIN_SRC_DIR}/latino/include)
  include_directories(${LLVM_BUILD_BINARY_DIR}/latino/include)
  include_directories(${LLVM_BUILD_BINARY_DIR}/latino/include)
endif()

message(STATUS "LLVM_INSTALL_PREFIX: ${LLVM_INSTALL_PREFIX}")
message(STATUS "LLVM_LIBRARY_DIR: ${LLVM_LIBRARY_DIR}")
link_directories(${LLVM_LIBRARY_DIR})

message(STATUS "LLVM_BUILD_MAIN_SRC_DIR: ${LLVM_BUILD_MAIN_SRC_DIR}")
message(STATUS "LLVM_BUILD_BINARY_DIR: ${LLVM_BUILD_BINARY_DIR}")

if(LLVM_INSTALL_PREFIX)
  include_directories(${LLVM_INSTALL_PREFIX}/tools/latino/include)
endif()

set(LLVM_MAIN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/llvm/include")
message(STATUS "LLVM_MAIN_INCLUDE_DIR: ${LLVM_MAIN_INCLUDE_DIR}")
include_directories(${LLVM_MAIN_INCLUDE_DIR})
if(WIN32)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/include/")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/llvm/include")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/clang/include")
  # to include .inc files
  # include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/tools/clang/include")
endif()
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(all)

add_subdirectory(utils/TableGen)

add_subdirectory(include)

# All targets below may depend on all tablegen'd files.
get_property(LATINO_TABLEGEN_TARGETS GLOBAL PROPERTY LATINO_TABLEGEN_TARGETS)
add_custom_target(latino-tablegen-targets
  DEPENDS
  omp_gen
  ${LATINO_TABLEGEN_TARGETS})
set_target_properties(latino-tablegen-targets PROPERTIES FOLDER "Misc")
list(APPEND LLVM_COMMON_DEPENDS latino-tablegen-targets)

# Force target to be built as soon as possible. Clang modules builds depend
# header-wise on it as they ship all headers from the umbrella folders. Building
# an entire module might include header, which depends on intrinsics_gen.
if(LLVM_ENABLE_MODULES AND NOT LATINO_BUILT_STANDALONE)
  list(APPEND LLVM_COMMON_DEPENDS intrinsics_gen)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

add_subdirectory(lib)
add_subdirectory(tools)

set(LATINO_MAIN_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LATINO_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to latino/lib")
set(ARG_ADDITIONAL_HEADERS)
message(STATUS "LATINO_MAIN_SRC_DIR: ${LATINO_MAIN_SRC_DIR}")
message(STATUS "LATINO_LIBRARY_DIR: ${LATINO_LIBRARY_DIR}")
message(STATUS "ARG_ADDITIONAL_HEADERS: ${ARG_ADDITIONAL_HEADERS}")

set(LATINO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "LATINO_SOURCE_DIR: ${LATINO_SOURCE_DIR}")

if(WIN32)
  set(GTEST_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
  set(GTEST_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/${CMAKE_BUILD_TYPE}/lib/gtest.lib)
  set(GTEST_MAIN_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-project/build/${CMAKE_BUILD_TYPE}/lib/gtest_main.lib)
  message(STATUS "GTEST_INCLUDE_DIR: ${GTEST_INCLUDE_DIR}")
  message(STATUS "GTEST_LIBRARY: ${GTEST_LIBRARY}")
  message(STATUS "GTEST_MAIN_LIBRARY: ${GTEST_MAIN_LIBRARY}")
endif()
if(LATINO_INCLUDE_TESTS)
  set(LLVM_BUILD_TESTS ON)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  message(STATUS "Found gtest: ${GTEST_INCLUDE_DIRS}, gtest/libs: ${GTEST_BOTH_LIBRARIES}")
  add_subdirectory(unittests)
endif()

message(STATUS "LLVM_TARGETS_TO_BUILD: ${LLVM_TARGETS_TO_BUILD}")
message(STATUS "LLVM_LIBDIR_SUFFIX: ${LLVM_LIBDIR_SUFFIX}")
message(STATUS "LLVM_LINK_COMPONENTS: ${LLVM_LINK_COMPONENTS}")
message(STATUS "COMPILE_FLAGS: ${COMPILE_FLAGS}")
