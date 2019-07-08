cmake_minimum_required(VERSION 3.14.0)

project(latino C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
if(WIN32)
  set(LLVM_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/Debug/bin")
endif()
set(LLVM_MAIN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/llvm")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/" "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/llvm/cmake/modules/" "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/llvm/tools/clang/cmake/modules/")
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

  set(LLVM_TOOLS_BINARY_DIR ${TOOLS_BINARY_DIR} CACHE PATH "Path to llvm/bin")
  set(LLVM_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to llvm/lib")
  set(LLVM_MAIN_INCLUDE_DIR ${INCLUDE_DIR} CACHE PATH "Path to llvm/include")
  set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT} CACHE PATH "Path to LLVM build tree")
  set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")

  file(TO_CMAKE_PATH ${LLVM_CONFIG_CMAKE_PATH} LLVM_CMAKE_PATH)

  find_program(LLVM_TABLEGEN_EXE "llvm-tblgen" ${LLVM_TOOLS_BINARY_DIR}
    NO_DEFAULT_PATH)
  #message(STATUS "LLVM_TOOLS_BINARY_DIR: ${LLVM_TOOLS_BINARY_DIR}")
  find_program(CLANG_TABLEGEN_EXE "clang-tblgen" ${LLVM_TOOLS_BINARY_DIR}
    NO_DEFAULT_PATH)
  #message(STATUS "CLANG_TABLEGEN_EXE: ${CLANG_TABLEGEN_EXE}")
  set(CLANG_TABLEGEN_TARGET ${CLANG_TABLEGEN_EXE})

  if(WIN32)
    set(LLVMCONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/cmake/modules/CMakeFiles/LLVMConfig.cmake")
  endif()
  if(EXISTS ${LLVMCONFIG_FILE})
    include(${LLVMCONFIG_FILE})
  else()
    message(FATAL_ERROR "Not found: ${LLVMCONFIG_FILE}")
  endif()
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
  if(WIN32 OR CYGWIN)
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_RUNTIME_OUTPUT_INTDIR})
  else()
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_LIBRARY_OUTPUT_INTDIR})
  endif()
endif()

option(LATINO_INCLUDE_TESTS "Generate build targets for the Latino unit tests." ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE) # Default to a Release build
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build [Release Debug]" FORCE)
endif()

#set(LLVM_TARGETS_TO_BUILD "all")
set(LLVM_TARGETS_TO_BUILD X86 CACHE STRING "")
set(LLVM_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/Debug/lib/)

include(AddLLVM)
include(TableGen)
include(AddClang)
include(AddLatino)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${CLANG_LIBS}")

set(LATINO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LATINO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(BACKEND_PACKAGE_STRING "LLVM ${LLVM_PACKAGE_VERSION}")

#Latino version
set(LATINO_VERSION_MAJOR 2)
set(LATINO_VERSION_MINOR 0)
set(LATINO_VERSION_PATCHLEVEL 0)
set(LATINO_VERSION "${LATINO_VERSION_MAJOR}.${LATINO_VERSION_MINOR}.${LATINO_VERSION_PATCHLEVEL}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/latino/Basic/Version.inc.in
${CMAKE_CURRENT_BINARY_DIR}/include/latino/Basic/Version.in)

if(LLVM_BUILD_MAIN_SRC_DIR)
  include_directories(${LLVM_BUILD_MAIN_SRC_DIR}/tools/clang/include)
  include_directories(${LLVM_BUILD_BINARY_DIR}/tools/clang/include)
endif()

#message(STATUS "LLVM_INSTALL_PREFIX: ${LLVM_INSTALL_PREFIX}")
#message(STATUS "LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS}")
link_directories(${LLVM_LIBRARY_DIRS})

set(LLVM_MAIN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/llvm/include")
include_directories(${LLVM_MAIN_INCLUDE_DIR})
if(WIN32)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/include/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/llvm/include")
endif()
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(all)

include_directories(include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(tools)

set(LATINO_MAIN_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LATINO_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to latino/lib")

set(ARG_ADDITIONAL_HEADERS)

if(WIN32)
  set(GTEST_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
  set(GTEST_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/Debug/lib/gtest.lib)
  set(GTEST_MAIN_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llvm-mirror/visualstudio/Debug/lib/gtest_main.lib)
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
message(STATUS "LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM_LIBDIR_SUFFIX: ${LLVM_LIBDIR_SUFFIX}")
message(STATUS "LLVM_LINK_COMPONENTS: ${LLVM_LINK_COMPONENTS}")
message(STATUS "COMPILE_FLAGS: ${COMPILE_FLAGS}")
