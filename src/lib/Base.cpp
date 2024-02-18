#include "Base.hpp"

#include <stdio.h>

#define EvalPrint(x) printf("%s = %d\n", #x, (int)(x))

void test() {
  printf("clang = %d\n", COMPILER_CLANG);
  printf("win32 = %d\n", OS_WINDOWS);
  printf("arch64 = %d\n", ARCH_X64);
  printf("arch86 = %d\n", ARCH_X86);

  U8 foo[100];
  for (U8 i = 0; i < ArrayCount(foo); i += 1) {
    foo[i] = i;
  }

  EvalPrint(ArrayCount(foo));

  EvalPrint(Min(1, 1));
  EvalPrint(Min(100, 10));
  EvalPrint(Max(1, 10));
  EvalPrint(Max(100, 10));
}