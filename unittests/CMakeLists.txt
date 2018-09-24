add_custom_target(LatinoUnitTests)
set_target_properties(LatinoUnitTests PROPERTIES FOLDER "Latino tests")

function(add_latino_unittest test_dirname)
  add_unittest(LatinoUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Basic)
#add_subdirectory(Lex)

# FIXME: libclang unit tests are disabled on Windows due
# to failures, mostly in libclang.VirtualFileOverlay_*.
#if(NOT WIN32 AND CLANG_TOOL_LIBCLANG_BUILD)
#  add_subdirectory(libclang)
#endif()
