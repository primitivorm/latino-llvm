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
  LLVMLTO
  LLVMPasses
  LLVMObjCARCOpts
  LLVMSymbolize
  LLVMDebugInfoPDB
  LLVMDebugInfoDWARF
  LLVMFuzzMutate
  LLVMTableGen
  LLVMDlltoolDriver
  LLVMLineEditor
  LLVMOrcJIT
  LLVMCoverage
  LLVMMIRParser
  LLVMObjectYAML
  LLVMLibDriver
  LLVMOption
  LLVMWindowsManifest
  LLVMX86Disassembler
  LLVMX86AsmParser
  LLVMX86CodeGen
  LLVMGlobalISel
  LLVMSelectionDAG
  LLVMAsmPrinter
  LLVMDebugInfoCodeView
  LLVMDebugInfoMSF
  LLVMX86Desc
  LLVMMCDisassembler
  LLVMX86Info
  LLVMX86AsmPrinter
  LLVMX86Utils
  LLVMMCJIT
  LLVMInterpreter
  LLVMExecutionEngine
  LLVMRuntimeDyld
  LLVMCodeGen
  LLVMTarget
  LLVMCoroutines
  LLVMipo
  LLVMInstrumentation
  LLVMVectorize
  LLVMScalarOpts
  LLVMLinker
  LLVMIRReader
  LLVMAsmParser
  LLVMInstCombine
  LLVMTransformUtils
  LLVMBitWriter
  LLVMAnalysis
  LLVMProfileData
  LLVMObject
  LLVMMCParser
  LLVMMC
  LLVMBitReader
  LLVMCore
  LLVMBinaryFormat
  LLVMSupport
  LLVMDemangle
)
