set(LLVM_LINK_COMPONENTS
  Support
  )

set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CheckerDependencyHandlingAnalyzerPlugin.exports)
add_llvm_library(CheckerDependencyHandlingAnalyzerPlugin MODULE BUILDTREE_ONLY CheckerDependencyHandling.cpp PLUGIN_TOOL clang)

latino_target_link_libraries(CheckerDependencyHandlingAnalyzerPlugin PRIVATE
  latinoAnalysis
  latinoAST
  latinoStaticAnalyzerCore
  latinoStaticAnalyzerFrontend
  )
