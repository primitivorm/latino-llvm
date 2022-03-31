add_custom_target(LatinoUnitTests)
set_target_properties(LatinoUnitTests PROPERTIES FOLDER "Latino tests")

if(CLANG_BUILT_STANDALONE)
  # LLVMTestingSupport library is needed for some of the unittests.
  if (EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Testing/Support
      AND NOT TARGET LLVMTestingSupport)
    add_subdirectory(${LLVM_MAIN_SRC_DIR}/lib/Testing/Support
      lib/Testing/Support)
  endif()
endif()

# add_clang_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against the clang
# Produces a binary named 'basename(test_dirname)'.
function(add_clang_unittest test_dirname)
  add_unittest(LatinoUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Lex)
add_subdirectory(ASTMatchers)