#include <stdio.h>
#include <gnu/libc-version.h>

int main(void)
{
  printf("GNU libc version (compile-time): %u.%u\n", __GLIBC__, __GLIBC_MINOR__);
  printf("GNU libc version (run-time): %s\n", gnu_get_libc_version());
  return 0;
}
