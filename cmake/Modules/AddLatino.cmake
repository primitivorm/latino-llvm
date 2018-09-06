macro(set_latino_windows_version_resource_properties name)
  if(DEFINED windows_resource_file)
    set_windows_version_resource_properties(${name} ${windows_resource_file}
    VERSION_MAJOR ${LATINO_VERSION_MAJOR}
    VERSION_MINOR ${LATINO_VERSION_MINOR}
    VERSION_PATCHLEVEL ${LATINO_VERSION_PATCHLEVEL}
    VERSION_STRING "${LATINO_VERSION} (${BACKEND_PACKAGE_STRING})"
    PRODUCT_NAME "latino")
  endif()
endmacro()

macro(add_latino_library name)
  cmake_parse_arguments(ARG
  "SHARED"
  ""
  "ADDITIONAL_HEADERS"
  ${ARGN})

  message(STATUS "add_latino_library. ARG_ADDITIONAL_HEADERS: ${ARG_ADDITIONAL_HEADERS}" )
  set(srcs)
  if(MSVC_IDE OR XCODE)
    file(RELATIVE_PATH lib_path
      ${LATINO_SOURCE_DIR}/lib/
      ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(NOT lib_path MATCHES "^[.][.]")
      file(GLOB_RECURSE headers
      ${LATINO_SOURCE_DIR}/include/latino/${lib_path}/*.h
      ${LATINO_SOURCE_DIR}/include/latino/${lib_path}/*.def)
      set_source_files_properties(${headers} PROPERTIES HEADER_FILE_ONLY ON)
      file(GLOB_RECURSE tds
      ${LATINO_SOURCE_DIR}/include/latino/${lib_path}/*.td)
      source_group("TableGen descriptions" FILES ${tds})
      set_source_files_properties(${tds} PROPERTIES HEADER_FILE_ONLY ON)
      if(headers OR tds)
        set(srcs ${headers} ${tds})
      endif()
    endif()
  endif(MSVC_IDE OR XCODE)
  if(srcs OR ARG_ADDITIONAL_HEADERS)
    set(srcs
    ARG_ADDITIONAL_HEADERS
    ${srcs}
    ${ARG_ADDITIONAL_HEADERS})
  endif()
  if(ARG_SHARED)
    set(ARG_ENABLE_SHARED)
  endif()
  llvm_add_library(${name} ${ARG_ENABLE_SHARED} ${ARG_UNPARSED_ARGUMENTS} ${srcs})
  if(TARGET ${name})
    target_link_libraries(${name} INTERFACE ${LLVM_COMMON_LIBS})
    if(NOT LLVM_INSTALL_TOOLCHAIN_ONLY OR ${name} STREQUAL "liblatino")
      if(${name} IN_LIST LLVM_DISTRIBUTION_COMPONENTS OR NOT LLVM_DISTRIBUTION_COMPONENTS)
        set(export_to_latinotargets EXPORT LatinoTargets)
        set_property(GLOBAL PROPERTY LATINO_HAS_EXPORTS True)
      endif()
      install(TARGETS ${name}
        COMPONENT ${name}
        ${export_to_latinotargets}
        LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX}
        ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX}
        RUNTIME DESTINATION bin)
      if(${ARG_SHARED} AND NOT CMAKE_CONFIGURATION_TYPES)
        add_custom_target(install-${name}
        DEPENDS ${name}
        COMMAND "${CMAKE_COMMAND}"
        -DCMAKE_INSTALL_COMPONENT=${name}
        -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
      endif()
    endif()
    set_property(GLOBAL APPEND PROPERTY LATINO_EXPORTS ${name})
  else()
    add_custom_target(${name})
  endif()
  set_target_properties(${name} PROPERTIES FOLDER "Latino libraries")
  set_latino_windows_version_resource_properties(${name})
endmacro(add_latino_library)
