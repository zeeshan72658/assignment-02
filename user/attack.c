#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  char *memory, *p, *limit;
  char prefix[] = "Here it is: ";
  int i, j, n;

  memory = sbrk(9 * 4096);
  if (memory == (char *)-1)
    exit(1);

  limit = memory + 9 * 4096;
  for (p = memory; p + sizeof(prefix) - 1 < limit; p++) {
    for (j = 0; prefix[j] != '\0' && p[j] == prefix[j]; j++)
      ;
    if (prefix[j] != '\0')
      continue;

    p += sizeof(prefix) - 1;
    n = 0;
    while (p + n < limit && p[n] != '\0')
      n++;
    if (p + n == limit)
      continue;
    for (i = 0; i < n; i++) {
      if (!((p[i] >= '0' && p[i] <= '9') ||
            (p[i] >= 'a' && p[i] <= 'z') ||
            (p[i] >= 'A' && p[i] <= 'Z'))) {
        n = 0;
        break;
      }
    }
    if (n > 0) {
      write(1, p, n);
      write(1, "\n", 1);
      exit(0);
    }
  }
  exit(1);
}
