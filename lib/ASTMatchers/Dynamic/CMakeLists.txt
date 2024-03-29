set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
)

# The registry source file ends up generating a lot of sections for each
# matcher. Each matcher appears to get a vtable and several methods. Each
# method needs .text, .pdata, .xdata, and .debug sections, adding to the
# section multiplier. By default MSVC has a 2^16 limit on the number of
# sections in an object file, and this needs more than that.
if (MSVC)
  set_source_files_properties(Registry.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_latino_library(latinoDynamicASTMatchers
  Diagnostics.cpp
  Marshallers.cpp
  Parser.cpp
  Registry.cpp
  VariantValue.cpp

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoBasic

  DEPENDS
  omp_gen
  )
