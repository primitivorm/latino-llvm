set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Analysis
  CodeGen
  Core
  IPO
  AggressiveInstCombine
  InstCombine
  Instrumentation
  MC
  MCParser
  ObjCARCOpts
  Option
  ScalarOpts
  Support
  TransformUtils
  Vectorize
  )

option(LATINO_PLUGIN_SUPPORT "Build clang with plugin support" ON)

# Support plugins.
if(LATINO_PLUGIN_SUPPORT)
  set(support_plugins SUPPORT_PLUGINS)
endif()

if(NOT LATINO_BUILT_STANDALONE)
  set(tablegen_deps intrinsics_gen)
endif()

add_latino_tool(latino
  driver.cpp
  cc1_main.cpp
  cc1as_main.cpp
  cc1gen_reproducer_main.cpp

  DEPENDS
  ${tablegen_deps}
  ${support_plugins}
  )

latino_target_link_libraries(latino
  PRIVATE
  latinoBasic
  latinoCodeGen
  latinoDriver
  latinoFrontend
  latinoFrontendTool
  latinoSerialization
  )

if(WIN32 AND NOT CYGWIN)
  # Prevent versioning if the buildhost is targeting for Win32.
else()
  set_target_properties(latino PROPERTIES VERSION ${LATINO_EXECUTABLE_VERSION})
endif()

# Support plugins.
if(LATINO_PLUGIN_SUPPORT)
  export_executable_symbols_for_plugins(latino)
endif()

add_dependencies(latino latino-resource-headers)

if(NOT LATINO_LINKS_TO_CREATE)
  set(LATINO_LINKS_TO_CREATE latino++ latino-cl latino-cpp)
endif()

foreach(link ${LATINO_LINKS_TO_CREATE})
  add_latino_symlink(${link} latino)
endforeach()

# Configure plist creation for OS X.
set (TOOL_INFO_PLIST "Info.plist" CACHE STRING "Plist name")
if (APPLE)
  if (LATINO_VENDOR)
    set(TOOL_INFO_NAME "${LATINO_VENDOR} latino")
  else()
    set(TOOL_INFO_NAME "clang")
  endif()

  set(TOOL_INFO_UTI "${LATINO_VENDOR_UTI}")
  set(TOOL_INFO_VERSION "${LATINO_VERSION}")
  set(TOOL_INFO_BUILD_VERSION "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}")

  set(TOOL_INFO_PLIST_OUT "${CMAKE_CURRENT_BINARY_DIR}/${TOOL_INFO_PLIST}")
  target_link_libraries(latino
    PRIVATE
    "-Wl,-sectcreate,__TEXT,__info_plist,${TOOL_INFO_PLIST_OUT}")
  configure_file("${TOOL_INFO_PLIST}.in" "${TOOL_INFO_PLIST_OUT}" @ONLY)

  set(TOOL_INFO_UTI)
  set(TOOL_INFO_NAME)
  set(TOOL_INFO_VERSION)
  set(TOOL_INFO_BUILD_VERSION)
endif()

if(LATINO_ORDER_FILE AND
    (LLVM_LINKER_IS_LD64 OR LLVM_LINKER_IS_GOLD OR LLVM_LINKER_IS_LLD))
  include(CheckLinkerFlag)

  if (LLVM_LINKER_IS_LD64)
    set(LINKER_ORDER_FILE_OPTION "-Wl,-order_file,${LATINO_ORDER_FILE}")
  elseif (LLVM_LINKER_IS_GOLD)
    set(LINKER_ORDER_FILE_OPTION "-Wl,--section-ordering-file,${LATINO_ORDER_FILE}")
  elseif (LLVM_LINKER_IS_LLD)
    set(LINKER_ORDER_FILE_OPTION "-Wl,--symbol-ordering-file,${LATINO_ORDER_FILE}")
  endif()

  # This is a test to ensure the actual order file works with the linker.
  check_linker_flag(${LINKER_ORDER_FILE_OPTION} LINKER_ORDER_FILE_WORKS)

  # Passing an empty order file disables some linker layout optimizations.
  # To work around this and enable workflows for re-linking when the order file
  # changes we check during configuration if the file is empty, and make it a
  # configuration dependency.
  file(READ ${LATINO_ORDER_FILE} ORDER_FILE LIMIT 20)
  if("${ORDER_FILE}" STREQUAL "\n")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LATINO_ORDER_FILE})
  elseif(LINKER_ORDER_FILE_WORKS)
    target_link_libraries(clang PRIVATE ${LINKER_ORDER_FILE_OPTION})
    set_target_properties(clang PROPERTIES LINK_DEPENDS ${LATINO_ORDER_FILE})
  endif()
endif()