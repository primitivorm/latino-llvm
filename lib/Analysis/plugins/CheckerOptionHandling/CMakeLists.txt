set(LLVM_LINK_COMPONENTS
  Support
  )

set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CheckerOptionHandlingAnalyzerPlugin.exports)
add_llvm_library(CheckerOptionHandlingAnalyzerPlugin MODULE BUILDTREE_ONLY CheckerOptionHandling.cpp PLUGIN_TOOL clang)

latino_target_link_libraries(CheckerOptionHandlingAnalyzerPlugin PRIVATE
  latinoAnalysis
  latinoAST
  latinoStaticAnalyzerCore
  latinoStaticAnalyzerFrontend
  )
