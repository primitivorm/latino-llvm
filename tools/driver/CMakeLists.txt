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

add_executable(latino driver.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  foreach(link_lib IN LISTS LIBRARY_LIST)
    target_link_libraries(${PROJECT_NAME} optimized ${link_lib})
    target_link_libraries(${PROJECT_NAME} debug     ${link_lib}d)
  endforeach()
else()
  target_link_libraries(${PROJECT_NAME} ${LIBRARY_LIST})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_LIBS}") # -std=c++11 -Wno-unused-parameter -fno-strict-aliasing -fno-exceptions -fno-rtti
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  latinoBasic
  LLVMSupport

  LLVMTarget
  LLVMXCoreCodeGen
  LLVMX86CodeGen
  LLVMSystemZCodeGen
  LLVMSparcCodeGen
  LLVMPowerPCCodeGen
  LLVMNVPTXCodeGen
  LLVMMSP430CodeGen
  LLVMMipsCodeGen
  LLVMLanaiCodeGen
  LLVMHexagonCodeGen
  LLVMBPFCodeGen
  LLVMARMCodeGen
  LLVMAMDGPUCodeGen
  LLVMAArch64CodeGen
  )

  #XCoreAsmPrinter
  #XCoreDesc
  #XCoreDisassembler
  #XCoreInfo
  #X86AsmParser
  #X86AsmPrinter
  #X86Info
