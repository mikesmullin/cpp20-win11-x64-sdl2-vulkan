
#pragma once

#include <iostream>  // IWYU pragma: keep

// Compiler context

#ifdef __clang__
#define COMPILER_CLANG 1
#else
#define COMPILER_CLANG 0
#error compiler not supported
#endif

#ifdef _WIN32
#define OS_WINDOWS 1
#else
#define OS_WINDOWS 0
#ifdef __linux__
#ifdef __ANDROID__
#define OS_LINUX 0
#define OS_ANDROID 1
#else
#define OS_ANDROID 0
#define OS_LINUX 1
#endif
#else
#define OS_LINUX 0
#ifdef __APPLE__
#define OS_MAC 1
#else
#define OS_MAC 0
#error os not supported
#endif
#endif
#endif

#ifdef __amd64__
#define ARCH_X64 1
#define ARCH_X86 0
#else
#define ARCH_X64 0
#ifdef __i386__
#define ARCH_X86 1
#define ARCH_X64 0
#else
#define ARCH_X86 0
#ifdef __aarch64__
#define ARCH_ARM64 1
#else
#define ARCH_ARM64 0
#error arch not supported
#endif
#endif
#endif

// Helpers

#define DEBUG_TRACE std::cout << "*** TRACE " << __FILE__ << ":" << __LINE__ << std::endl;

#ifndef ENABLE_ASSERT
#define ENABLE_ASSERT 1
#endif

#ifndef AssertBreak
#define AssertBreak() (*(int*)0 = 0)
#endif

#if ENABLE_ASSERT
#define Assert(c)  \
  if (!(c)) {      \
    AssertBreak(); \
  }
#else
#define Assert(c)
#endif

#define Stringify(S) #S
#define Glue(A, B) A##B

#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))
#define VectorSize(a) sizeof(a[0]) * a.size()

#define IntFromPtr(p) (unsigned long long)((char*)p - (char*)0)
#define PtrFromInt(n) (void*)((char*)0 + (n))

#define Member(T, m) (((T*)0)->m)
#define OffsetOfMember(T, m) IntFromPtr(&Member(T, m))

// Math

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(min, n, max) (((n) < (min)) ? (min) : ((max) < (n)) ? (max) : (n))

// Scalars

#include <stdint.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef s8 b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;
typedef float f32;
typedef double f64;

typedef void VoidFunc(void);

// TODO: Vector lib (replacing GLM)

void test();