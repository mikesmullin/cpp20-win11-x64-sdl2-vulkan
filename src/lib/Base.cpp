#include "Base.hpp"

#include <stdio.h>

void test() {
  printf("clang = %d\n", COMPILER_CLANG);
  printf("win32 = %d\n", OS_WINDOWS);
  printf("arch64 = %d\n", ARCH_X64);
  printf("arch86 = %d\n", ARCH_X86);
}