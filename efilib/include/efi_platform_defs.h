/** @file
  Precise Time functions library

  Copyright (c) 2026, DTI Technologies s.r.o. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _EFI_PLATFORM_DEFS_H_
#define _EFI_PLATFORM_DEFS_H_

//
// Identify compiler
//
#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC
#elif defined(_MSC_VER)
#define COMPILER_MSC
#define COMPILER_MSVC
#else
#error "Compiler detection failed!"
#endif

//
// Include needed headers
//

#if defined(COMPILER_MSVC)
#include <stdint.h>
#include <intrin.h>
#elif defined(COMPILER_GCC)
#include <stdint.h>
#include <stddef.h>
#endif

//
// Identify platform and set platform macros
//
#ifdef _WIN32
  #if _M_X64 || _M_IA64
    #define MACHINE64
    #define MACHINE_64
    #define MACHINE_X86
    #define MACHINE_X86_64
  #elif _M_ARM
    #define MACHINE32
    #define MACHINE_32
    #define MACHINE_ARM
    #define MACHINE_ARM32
  #elif _M_ARM64
    #define MACHINE64
    #define MACHINE_64
    #define MACHINE_ARM
    #define MACHINE_ARM64
  #elif _M_IX86
    #define MACHINE32
    #define MACHINE_32
    #define MACHINE_X86
    #define MACHINE_X86_32
  #endif
#else
  #if defined(__aarch64__)
    #define MACHINE64
    #define MACHINE_64
    #define MACHINE_ARM
    #define MACHINE_ARM64
  #elif defined(__arm__)
    #define MACHINE32
    #define MACHINE_32
    #define MACHINE_ARM
    #define MACHINE_ARM32
  #elif defined(__x86_64__)
    #define MACHINE64
    #define MACHINE_64
    #define MACHINE_X86
    #define MACHINE_X86_64
  #elif defined(__i386__)
    #define MACHINE32
    #define MACHINE_32
    #define MACHINE_X86
    #define MACHINE_X86_32
  #endif
#endif

//
// Prepare macros for variable argument lists
//
#define _INT_SIZE_OF(n)                     ((sizeof(n) + sizeof(UINTN) - 1) &~(sizeof(UINTN) - 1))

#if defined(COMPILER_MSC)
#include <stdarg.h>
#endif

#if defined(_M_ARM64)
typedef char *VA_LIST;
#define VA_START(ap, x)                     __va_start(&ap, &x, _INT_SIZE_OF(x), __alignof(x), &x)
#define VA_ARG(ap, t)                       (*(TYPE *)((ap += _INT_SIZE_OF(t) + ((-(INTN)Marker) & (sizeof(t) - 1))) - _INT_SIZE_OF(t)))
#define VA_END(ap)                          (ap = (VA_LIST)0)
#define VA_COPY(dst, src)                   ((void)((dst) = (src)))
//#elif defined(__GNUC__) || defined(__clang__)
//typedef __builtin_ms_va_list                VA_LIST;
//#define VA_START(ap, x)                     __builtin_ms_va_start(ap, x)
//#define VA_ARG(ap, t)                       ((sizeof(t) < sizeof(UINTN)) ? (t)(__builtin_va_arg(ap, UINTN)) : (t)(__builtin_va_arg(ap, t)))
//#define VA_END(ap)                          __builtin_ms_va_end(ap)
//#define VA_COPY(dst, src)                   __builtin_ms_va_copy(dst, src)
#else
#define VA_LIST                             va_list
#define VA_START                            va_start
#define VA_ARG                              va_arg
#define VA_END                              va_end
#define VA_COPY                             va_copy
#endif

//
// Prepare macros for string formating
//
#ifndef PRIu8
#define PRIu8                "hhu"
#endif
#ifndef PRIu16
#define PRIu16               "hu"
#endif
#ifndef PRIu32
#define PRIu32               "lu"
#endif
#ifndef PRIu64
#define PRIu64               "llu"
#endif
#ifndef PRIuPTR
#define PRIuPTR              "Id"
#endif

#ifndef PRId8
#define PRId8                "hhd"
#endif
#ifndef PRId16
#define PRId16               "hd"
#endif
#ifndef PRId32
#define PRId32               "ld"
#endif
#ifndef PRId64
#define PRId64               "lld"
#endif
#ifndef PRIdPTR
#define PRIdPTR              "Iu"
#endif

#ifndef PRIx8
#define PRIx8                "hhx"
#endif
#ifndef PRIx16
#define PRIx16               "hx"
#endif
#ifndef PRIx32
#define PRIx32               "lx"
#endif
#ifndef PRIx64
#define PRIx64               "llx"
#endif
#ifndef PRIxPTR
#define PRIxPTR              "Ix"
#endif

#ifndef PRIX8
#define PRIX8                "hhX"
#endif
#ifndef PRIX16
#define PRIX16               "hX"
#endif
#ifndef PRIX32
#define PRIX32               "lX"
#endif
#ifndef PRIX64
#define PRIX64               "llX"
#endif
#ifndef PRIXPTR
#define PRIXPTR              "IX"
#endif

#ifndef PRI_FLOAT
#define PRI_FLOAT            "f"
#endif
#ifndef PRI_DOUBLE
#ifdef COMPILER_MSC
#define PRI_DOUBLE           "f"
#else
#define PRI_DOUBLE           "lf"
#endif
#endif
#ifndef PRI_LDBL
#define PRI_LDBL             "Lf"
#endif

#ifndef PRIuN
#ifdef MACHINE_64
#define PRIuN                PRIu64
#else
#define PRIuN                PRIu32
#endif
#endif

#ifndef PRIdN
#ifdef MACHINE_64
#define PRIdN                PRId64
#else
#define PRIdN                PRId32
#endif
#endif

#ifndef PRIxN
#ifdef MACHINE_64
#define PRIxN                PRIx64
#else
#define PRIxN                PRIx32
#endif
#endif

#ifndef PRIXN
#ifdef MACHINE_64
#define PRIXN                PRIX64
#else
#define PRIXN                PRIX32
#endif
#endif

//
// Define macro for newline
//
#define NEWLINE              "\n"

//
// Define macros for branch prediction control
//
#ifdef _WIN32
#define LIKELY(x)            x
#define UNLIKELY(x)          x
#else
#define LIKELY(x)            __builtin_expect(!!(x),1)
#define UNLIKELY(x)          __builtin_expect(!!(x),0)
#endif

#endif
