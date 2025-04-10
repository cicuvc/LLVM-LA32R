add_subdirectory(memory_utils)

if(LIBC_CONF_STRING_UNSAFE_WIDE_READ)
  list(APPEND string_config_options "-DLIBC_COPT_STRING_UNSAFE_WIDE_READ")
endif()
if(LIBC_CONF_MEMSET_X86_USE_SOFTWARE_PREFETCHING)
  list(APPEND string_config_options "-DLIBC_COPT_MEMSET_X86_USE_SOFTWARE_PREFETCHING")
endif()
if(string_config_options)
  list(PREPEND string_config_options "COMPILE_OPTIONS")
endif()

add_header_library(
  string_utils
  HDRS
    string_utils.h
  DEPENDS
    .memory_utils.inline_bzero
    .memory_utils.inline_memcpy
    libc.include.stdlib
    libc.src.__support.common
    libc.src.__support.CPP.bitset
  ${string_config_options}
)

add_header_library(
  allocating_string_utils
  HDRS
    allocating_string_utils.h
  DEPENDS
    .memory_utils.inline_memcpy
    libc.include.stdlib
    libc.src.__support.CPP.optional
    libc.src.__support.macros.config
)

add_entrypoint_object(
  bcopy
  SRCS
    bcopy.cpp
  HDRS
    bcopy.h
)

add_entrypoint_object(
  index
  SRCS
    index.cpp
  HDRS
    index.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  memccpy
  SRCS
    memccpy.cpp
  HDRS
    memccpy.h
)

add_entrypoint_object(
  mempcpy
  SRCS
    mempcpy.cpp
  HDRS
    mempcpy.h
  DEPENDS
    .memory_utils.inline_memcpy
)

add_entrypoint_object(
  memmem
  SRCS
    memmem.cpp
  HDRS
    memmem.h
  DEPENDS
    .memory_utils.inline_memmem
)

add_entrypoint_object(
  memchr
  SRCS
    memchr.cpp
  HDRS
    memchr.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  memrchr
  SRCS
    memrchr.cpp
  HDRS
    memrchr.h
)

add_entrypoint_object(
  rindex
  SRCS
    rindex.cpp
  HDRS
    rindex.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  stpcpy
  SRCS
    stpcpy.cpp
  HDRS
    stpcpy.h
  DEPENDS
    .mempcpy
    .string_utils
)

add_entrypoint_object(
  stpncpy
  SRCS
    stpncpy.cpp
  HDRS
    stpncpy.h
  DEPENDS
    .memory_utils.inline_bzero
)

add_entrypoint_object(
  strcat
  SRCS
    strcat.cpp
  HDRS
    strcat.h
  DEPENDS
    .strcpy
    .string_utils
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strchr
  SRCS
    strchr.cpp
  HDRS
    strchr.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strchrnul
  SRCS
    strchrnul.cpp
  HDRS
    strchrnul.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strcmp
  SRCS
    strcmp.cpp
  HDRS
    strcmp.h
  DEPENDS
    .memory_utils.inline_strcmp
)

add_entrypoint_object(
  strcasecmp
  SRCS
    strcasecmp.cpp
  HDRS
    strcasecmp.h
  DEPENDS
    .memory_utils.inline_strcmp
    libc.src.__support.ctype_utils
)

add_entrypoint_object(
  strcasestr
  SRCS
    strcasestr.cpp
  HDRS
    strcasestr.h
  DEPENDS
    .memory_utils.inline_strstr
    libc.src.__support.ctype_utils
)

add_entrypoint_object(
  strcoll
  SRCS
    strcoll.cpp
  HDRS
    strcoll.h
)

add_entrypoint_object(
  strcoll_l
  SRCS
    strcoll_l.cpp
  HDRS
  strcoll_l.h
)

add_entrypoint_object(
  strcpy
  SRCS
    strcpy.cpp
  HDRS
    strcpy.h
  DEPENDS
    .memory_utils.inline_memcpy
    .string_utils
)

add_entrypoint_object(
  strcspn
  SRCS
    strcspn.cpp
  HDRS
    strcspn.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strdup
  SRCS
    strdup.cpp
  HDRS
    strdup.h
  DEPENDS
    .memory_utils.inline_memcpy
    .string_utils
    libc.include.stdlib
    libc.src.errno.errno
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strerror
  SRCS
    strerror.cpp
  HDRS
    strerror.h
  DEPENDS
    libc.src.__support.StringUtil.error_to_string
)

add_entrypoint_object(
  strerror_r
  SRCS
    strerror_r.cpp
  HDRS
    strerror_r.h
  DEPENDS
    libc.src.__support.StringUtil.error_to_string
)

add_entrypoint_object(
  strlcat
  SRCS
    strlcat.cpp
  HDRS
    strlcat.h
  DEPENDS
    .string_utils
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strlcpy
  SRCS
    strlcpy.cpp
  HDRS
    strlcpy.h
  DEPENDS
    .string_utils
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strlen
  SRCS
    strlen.cpp
  HDRS
    strlen.h
  DEPENDS
    .string_utils
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strncat
  SRCS
    strncat.cpp
  HDRS
    strncat.h
  DEPENDS
    .strncpy
    .string_utils
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strncmp
  SRCS
    strncmp.cpp
  HDRS
    strncmp.h
  DEPENDS
    .memory_utils.inline_strcmp
)

add_entrypoint_object(
  strncasecmp
  SRCS
    strncasecmp.cpp
  HDRS
    strncasecmp.h
  DEPENDS
    .memory_utils.inline_strcmp
    libc.src.__support.ctype_utils
)

add_entrypoint_object(
  strncpy
  SRCS
    strncpy.cpp
  HDRS
    strncpy.h
)

add_entrypoint_object(
  strndup
  SRCS
    strndup.cpp
  HDRS
    strndup.h
  DEPENDS
    .memory_utils.inline_memcpy
    .string_utils
    libc.include.stdlib
    libc.src.__support.CPP.new
    libc.include.llvm-libc-types.size_t
)

add_entrypoint_object(
  strnlen
  SRCS
    strnlen.cpp
  HDRS
    strnlen.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strpbrk
  SRCS
    strpbrk.cpp
  HDRS
    strpbrk.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strrchr
  SRCS
    strrchr.cpp
  HDRS
    strrchr.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strsep
  SRCS
    strsep.cpp
  HDRS
    strsep.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strsignal
  SRCS
    strsignal.cpp
  HDRS
    strsignal.h
  DEPENDS
    libc.src.__support.StringUtil.signal_to_string
)

add_entrypoint_object(
  strspn
  SRCS
    strspn.cpp
  HDRS
    strspn.h
  DEPENDS
    libc.src.__support.CPP.bitset
)

add_entrypoint_object(
  strstr
  SRCS
    strstr.cpp
  HDRS
    strstr.h
  DEPENDS
    .memory_utils.inline_strstr
)

add_entrypoint_object(
  strtok
  SRCS
    strtok.cpp
  HDRS
    strtok.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strtok_r
  SRCS
    strtok_r.cpp
  HDRS
    strtok_r.h
  DEPENDS
    .string_utils
)

add_entrypoint_object(
  strxfrm
  SRCS
    strxfrm.cpp
  HDRS
    strxfrm.h
  DEPENDS
    .string_utils
    .memory_utils.inline_memcpy
)

add_entrypoint_object(
  strxfrm_l
  SRCS
    strxfrm_l.cpp
  HDRS
    strxfrm_l.h
  DEPENDS
    .string_utils
    .memory_utils.inline_memcpy
)

add_entrypoint_object(
  memset_explicit
  SRCS
    memset_explicit.cpp
  HDRS
    memset_explicit.h
  DEPENDS
    .string_utils
    .memory_utils.inline_memset
)

# Helper to define a function with multiple implementations
# - Computes flags to satisfy required/rejected features and arch,
# - Declares an entry point,
# - Attach the REQUIRE_CPU_FEATURES property to the target,
# - Add the fully qualified target to `${name}_implementations` global property for tests.
function(add_implementation name impl_name)
  cmake_parse_arguments(
    "ADD_IMPL"
    "" # Optional arguments
    "" # Single value arguments
    "REQUIRE;SRCS;HDRS;DEPENDS;COMPILE_OPTIONS;MLLVM_COMPILE_OPTIONS" # Multi value arguments
    ${ARGN})

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # Note that '-mllvm' needs to be prefixed with 'SHELL:' to prevent CMake flag deduplication.
    foreach(opt IN LISTS ADD_IMPL_MLLVM_COMPILE_OPTIONS)
      list(APPEND ADD_IMPL_COMPILE_OPTIONS "SHELL:-mllvm ${opt}")
    endforeach()
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    # Prevent warning when passing x86 SIMD types as template arguments.
    # e.g. "warning: ignoring attributes on template argument ‘__m128i’ [-Wignored-attributes]"
    list(APPEND ADD_IMPL_COMPILE_OPTIONS "-Wno-ignored-attributes")
  endif()

  add_entrypoint_object(${impl_name}
    NAME ${name}
    SRCS ${ADD_IMPL_SRCS}
    HDRS ${ADD_IMPL_HDRS}
    DEPENDS ${ADD_IMPL_DEPENDS}
    COMPILE_OPTIONS -O3 ${ADD_IMPL_COMPILE_OPTIONS}
  )
  get_fq_target_name(${impl_name} fq_target_name)
  set_target_properties(${fq_target_name} PROPERTIES REQUIRE_CPU_FEATURES "${ADD_IMPL_REQUIRE}")
  set_property(GLOBAL APPEND PROPERTY "${name}_implementations" "${fq_target_name}")
endfunction()

# ------------------------------------------------------------------------------
# bcmp
# ------------------------------------------------------------------------------

function(add_bcmp bcmp_name)
  add_implementation(bcmp ${bcmp_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/bcmp.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/bcmp.h
    DEPENDS
      .memory_utils.memory_utils
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_bcmp(bcmp_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_bcmp(bcmp_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_bcmp(bcmp_x86_64_opt_avx2   COMPILE_OPTIONS -march=haswell        REQUIRE AVX2)
  add_bcmp(bcmp_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512BW)
  add_bcmp(bcmp_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_bcmp(bcmp)
elseif(LIBC_TARGET_OS_IS_GPU)
  add_bcmp(bcmp)
else()
  add_bcmp(bcmp_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_bcmp(bcmp)
endif()

# ------------------------------------------------------------------------------
# bzero
# ------------------------------------------------------------------------------

function(add_bzero bzero_name)
  add_implementation(bzero ${bzero_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/bzero.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/bzero.h
    DEPENDS
      .memory_utils.inline_memset
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_bzero(bzero_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_bzero(bzero_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_bzero(bzero_x86_64_opt_avx2   COMPILE_OPTIONS -march=haswell        REQUIRE AVX2)
  add_bzero(bzero_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512F)
  add_bzero(bzero_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_bzero(bzero)
elseif(LIBC_TARGET_OS_IS_GPU)
  add_bzero(bzero)
else()
  add_bzero(bzero_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_bzero(bzero)
endif()

# ------------------------------------------------------------------------------
# memcmp
# ------------------------------------------------------------------------------

function(add_memcmp memcmp_name)
  add_implementation(memcmp ${memcmp_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/memcmp.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/memcmp.h
    DEPENDS
      .memory_utils.inline_memcmp
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_memcmp(memcmp_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_memcmp(memcmp_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_memcmp(memcmp_x86_64_opt_avx2   COMPILE_OPTIONS -march=haswell        REQUIRE AVX2)
  add_memcmp(memcmp_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512BW)
  add_memcmp(memcmp_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memcmp(memcmp)
elseif(${LIBC_TARGET_ARCHITECTURE_IS_AARCH64})
  add_memcmp(memcmp_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memcmp(memcmp)
elseif(LIBC_TARGET_OS_IS_GPU)
  add_memcmp(memcmp)
else()
  add_memcmp(memcmp_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memcmp(memcmp)
endif()

# ------------------------------------------------------------------------------
# memcpy
# ------------------------------------------------------------------------------

function(add_memcpy memcpy_name)
  add_implementation(memcpy ${memcpy_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/memcpy.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/memcpy.h
    DEPENDS
      .memory_utils.inline_memcpy
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_memcpy(memcpy_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_memcpy(memcpy_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_memcpy(memcpy_x86_64_opt_avx    COMPILE_OPTIONS -march=sandybridge    REQUIRE AVX)
  add_memcpy(memcpy_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512F)
  add_memcpy(memcpy_x86_64_opt_sw_prefetch_sse4   COMPILE_OPTIONS -DLIBC_COPT_MEMCPY_X86_USE_SOFTWARE_PREFETCHING -march=nehalem        REQUIRE SSE4_2)
  add_memcpy(memcpy_x86_64_opt_sw_prefetch_avx    COMPILE_OPTIONS -DLIBC_COPT_MEMCPY_X86_USE_SOFTWARE_PREFETCHING -march=sandybridge    REQUIRE AVX)
  add_memcpy(memcpy_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memcpy(memcpy)
elseif(${LIBC_TARGET_ARCHITECTURE_IS_AARCH64})
  # Disable tail merging as it leads to lower performance.
  add_memcpy(memcpy_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE}
                                      MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
  add_memcpy(memcpy                   MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
elseif(LIBC_TARGET_OS_IS_GPU)
  add_memcpy(memcpy)
else()
  add_memcpy(memcpy_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memcpy(memcpy)
endif()

# ------------------------------------------------------------------------------
# memmove
# ------------------------------------------------------------------------------

function(add_memmove memmove_name)
  add_implementation(memmove ${memmove_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/memmove.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/memmove.h
    DEPENDS
      .memory_utils.inline_memcpy
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_memmove(memmove_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_memmove(memmove_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_memmove(memmove_x86_64_opt_avx2   COMPILE_OPTIONS -march=haswell        REQUIRE AVX2)
  add_memmove(memmove_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512F)
  add_memmove(memmove_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memmove(memmove)
elseif(${LIBC_TARGET_ARCHITECTURE_IS_AARCH64})
  # Disable tail merging as it leads to lower performance.
  add_memmove(memmove_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE}
                                        MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
  add_memmove(memmove                   MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
elseif(LIBC_TARGET_OS_IS_GPU)
  add_memmove(memmove)
else()
  add_memmove(memmove_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memmove(memmove)
endif()

# ------------------------------------------------------------------------------
# memset
# ------------------------------------------------------------------------------

function(add_memset memset_name)
  add_implementation(memset ${memset_name}
    SRCS ${LIBC_SOURCE_DIR}/src/string/memset.cpp
    HDRS ${LIBC_SOURCE_DIR}/src/string/memset.h
    DEPENDS
      .memory_utils.inline_memset
      libc.include.string
    ${ARGN}
  )
endfunction()

if(${LIBC_TARGET_ARCHITECTURE_IS_X86})
  add_memset(memset_x86_64_opt_sse2   COMPILE_OPTIONS -march=k8             REQUIRE SSE2)
  add_memset(memset_x86_64_opt_sse4   COMPILE_OPTIONS -march=nehalem        REQUIRE SSE4_2)
  add_memset(memset_x86_64_opt_avx2   COMPILE_OPTIONS -march=haswell        REQUIRE AVX2)
  add_memset(memset_x86_64_opt_avx512 COMPILE_OPTIONS -march=skylake-avx512 REQUIRE AVX512F)
  add_memset(memset_x86_64_opt_sw_prefetch COMPILE_OPTIONS -DLIBC_COPT_MEMSET_X86_USE_SOFTWARE_PREFETCHING)
  add_memset(memset_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memset(memset)
elseif(${LIBC_TARGET_ARCHITECTURE_IS_AARCH64})
  # Disable tail merging as it leads to lower performance.
  add_memset(memset_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE}
                                      MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
  add_memset(memset                   MLLVM_COMPILE_OPTIONS "-tail-merge-threshold=0")
elseif(LIBC_TARGET_OS_IS_GPU)
  add_memset(memset)
else()
  add_memset(memset_opt_host          COMPILE_OPTIONS ${LIBC_COMPILE_OPTIONS_NATIVE})
  add_memset(memset)
endif()
