create_subdirectory_options(CLANG TOOL)

# add_latino_subdirectory(diagtool)
add_latino_subdirectory(driver)
# add_latino_subdirectory(clang-diff)
# add_latino_subdirectory(clang-format)
# add_latino_subdirectory(clang-format-vs)
# add_latino_subdirectory(clang-fuzzer)
# add_latino_subdirectory(clang-import-test)
# add_latino_subdirectory(clang-offload-bundler)
# add_latino_subdirectory(clang-offload-wrapper)
# add_latino_subdirectory(clang-scan-deps)

# add_latino_subdirectory(c-index-test)

# add_latino_subdirectory(clang-rename)
# add_latino_subdirectory(clang-refactor)
# if(UNIX)
#   add_latino_subdirectory(clang-shlib)
# endif()

# if(CLANG_ENABLE_ARCMT)
#   add_latino_subdirectory(arcmt-test)
#   add_latino_subdirectory(c-arcmt-test)
# endif()

# if(CLANG_ENABLE_STATIC_ANALYZER)
#   add_latino_subdirectory(clang-check)
#   add_latino_subdirectory(clang-extdef-mapping)
#   add_latino_subdirectory(scan-build)
#   add_latino_subdirectory(scan-view)
# endif()

# We support checking out the clang-tools-extra repository into the 'extra'
# subdirectory. It contains tools developed as part of the Clang/LLVM project
# on top of the Clang tooling platform. We keep them in a separate repository
# to keep the primary Clang repository small and focused.
# It also may be included by LLVM_EXTERNAL_CLANG_TOOLS_EXTRA_SOURCE_DIR.
# add_llvm_external_project(clang-tools-extra extra)

# libclang may require clang-tidy in clang-tools-extra.
# add_latino_subdirectory(libclang)
