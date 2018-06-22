add_custom_target(LatinoUnitTests)
set_target_properties(LatinoUnitTests PROPERTIES FOLDER "Latino tests")

function(add_latino_unittest test_dirname)
  add_unittest(LatinoUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Basic)
add_subdirectory(Lex)
