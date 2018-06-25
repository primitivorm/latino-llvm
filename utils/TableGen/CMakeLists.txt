set(LLVM_LINK_COMPONENTS Support)

add_tablegen(latino-tblgen LATINO
TableGen.cpp
)

set_target_properties(latino-tblgen PROPERTIES "Latino tablegenning")