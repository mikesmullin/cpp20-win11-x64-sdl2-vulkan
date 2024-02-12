#pragma once

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
#error os not supported
#endif

#ifdef __amd64__
#define ARCH_X64 1
#define ARCH_X86 0
#elifdef __i386__
#define ARCH_X86 1
#define ARCH_X64 0
#else
#define ARCH_X86 0
#define ARCH_X64 0
#error arch not supported
#endif

void test();