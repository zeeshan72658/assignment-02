#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  int offset, i, size;
  uint32 value32;
  uint16 value16;
  uint64 value64;
  char *string;

  offset = 0;
  for (i = 0; fmt[i] != '\0'; i++) {
    switch (fmt[i]) {
    case 'i': size = sizeof(value32); break;
    case 'p':
    case 's': size = sizeof(value64); break;
    case 'h': size = sizeof(value16); break;
    case 'c': size = 1; break;
    case 'S':
      while (offset < len && data[offset] != '\0')
        printf("%c", data[offset++]);
      printf("\n");
      return;
    default:
      continue;
    }

    if (len - offset < size) {
      printf("memdump: not enough data for '%c'\n", fmt[i]);
      return;
    }

    switch (fmt[i]) {
    case 'i':
      memmove(&value32, data + offset, sizeof(value32));
      printf("%d\n", (int)value32);
      break;
    case 'p':
      memmove(&value64, data + offset, sizeof(value64));
      printf("%lx\n", value64);
      break;
    case 'h':
      memmove(&value16, data + offset, sizeof(value16));
      printf("%d\n", (short)value16);
      break;
    case 'c':
      printf("%c\n", data[offset]);
      break;
    case 's':
      memmove(&value64, data + offset, sizeof(value64));
      string = (char *)(uint64)value64;
      printf("%s\n", string);
      break;
    }
    offset += size;
  }
}
