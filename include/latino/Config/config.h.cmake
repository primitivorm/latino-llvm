/* This generated file is for internal use. Do not include it from headers. */

#ifdef LATINO_CONFIG_H
#error config.h can only be included once
#else
#define LATINO_CONFIG_H

/* Bug report URL. */
#define BUG_REPORT_URL "${BUG_REPORT_URL}"

/* Default linker to use. */
#define LATINO_DEFAULT_LINKER "${LATINO_DEFAULT_LINKER}"

/* Default C/ObjC standard to use. */
#cmakedefine LATINO_DEFAULT_STD_C LangStandard::lang_${LATINO_DEFAULT_STD_C}

/* Default C++/ObjC++ standard to use. */
#cmakedefine LATINO_DEFAULT_STD_CXX LangStandard::lang_${LATINO_DEFAULT_STD_CXX}

/* Default C++ stdlib to use. */
#define LATINO_DEFAULT_CXX_STDLIB "${LATINO_DEFAULT_CXX_STDLIB}"

/* Default runtime library to use. */
#define LATINO_DEFAULT_RTLIB "${LATINO_DEFAULT_RTLIB}"

/* Default unwind library to use. */
#define LATINO_DEFAULT_UNWINDLIB "${LATINO_DEFAULT_UNWINDLIB}"

/* Default objcopy to use */
#define LATINO_DEFAULT_OBJCOPY "${LATINO_DEFAULT_OBJCOPY}"

/* Default OpenMP runtime used by -fopenmp. */
#define LATINO_DEFAULT_OPENMP_RUNTIME "${LATINO_DEFAULT_OPENMP_RUNTIME}"

/* Default architecture for OpenMP offloading to Nvidia GPUs. */
#define LATINO_OPENMP_NVPTX_DEFAULT_ARCH "${LATINO_OPENMP_NVPTX_DEFAULT_ARCH}"

/* Default architecture for SystemZ. */
#define LATINO_SYSTEMZ_DEFAULT_ARCH "${LATINO_SYSTEMZ_DEFAULT_ARCH}"

/* Multilib suffix for libdir. */
#define LATINO_LIBDIR_SUFFIX "${LATINO_LIBDIR_SUFFIX}"

/* Relative directory for resource files */
#define LATINO_RESOURCE_DIR "${LATINO_RESOURCE_DIR}"

/* Directories clang will search for headers */
#define C_INCLUDE_DIRS "${C_INCLUDE_DIRS}"

/* Directories clang will search for configuration files */
#cmakedefine LATINO_CONFIG_FILE_SYSTEM_DIR "${LATINO_CONFIG_FILE_SYSTEM_DIR}"
#cmakedefine LATINO_CONFIG_FILE_USER_DIR "${LATINO_CONFIG_FILE_USER_DIR}"

/* Default <path> to all compiler invocations for --sysroot=<path>. */
#define DEFAULT_SYSROOT "${DEFAULT_SYSROOT}"

/* Directory where gcc is installed. */
#define GCC_INSTALL_PREFIX "${GCC_INSTALL_PREFIX}"

/* Define if we have libxml2 */
#cmakedefine LATINO_HAVE_LIBXML ${LATINO_HAVE_LIBXML}

/* Define if we have sys/resource.h (rlimits) */
#cmakedefine LATINO_HAVE_RLIMITS ${LATINO_HAVE_RLIMITS}

/* The LLVM product name and version */
#define BACKEND_PACKAGE_STRING "${BACKEND_PACKAGE_STRING}"

/* Linker version detected at compile time. */
#cmakedefine HOST_LINK_VERSION "${HOST_LINK_VERSION}"

/* pass --build-id to ld */
#cmakedefine ENABLE_LINKER_BUILD_ID

/* enable x86 relax relocations by default */
#cmakedefine01 ENABLE_X86_RELAX_RELOCATIONS

/* Enable the experimental new pass manager by default */
#cmakedefine01 ENABLE_EXPERIMENTAL_NEW_PASS_MANAGER

/* Enable each functionality of modules */
#cmakedefine01 LATINO_ENABLE_ARCMT
#cmakedefine01 LATINO_ENABLE_OBJC_REWRITER
#cmakedefine01 LATINO_ENABLE_STATIC_ANALYZER

/* Spawn a new process clang.exe for the CC1 tool invocation, when necessary */
#cmakedefine01 LATINO_SPAWN_CC1

#endif
