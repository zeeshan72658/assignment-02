#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int
is_digit(char c)
{
  return c >= '0' && c <= '9';
}

static int
is_separator(char c)
{
  return c == ' ' || c == '-' || c == '\r' || c == '\t' ||
         c == '\n' || c == '.' || c == '/' || c == ',';
}

static void
scan(int fd)
{
  char buf[128];
  int n, i, in_token, is_number;
  uint64 value;

  in_token = 0;
  is_number = 1;
  value = 0;
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    for (i = 0; i < n; i++) {
      if (is_separator(buf[i])) {
        if (in_token && is_number && (value % 5 == 0 || value % 6 == 0))
          printf("%lu\n", value);
        in_token = 0;
        is_number = 1;
        value = 0;
      } else if (is_digit(buf[i])) {
        if (!in_token) {
          in_token = 1;
          is_number = 1;
          value = 0;
        }
        if (is_number)
          value = value * 10 + (buf[i] - '0');
      } else {
        if (!in_token) {
          in_token = 1;
          value = 0;
        }
        is_number = 0;
      }
    }
  }

  if (in_token && is_number && (value % 5 == 0 || value % 6 == 0))
    printf("%lu\n", value);
}

int
main(int argc, char *argv[])
{
  int i, fd;

  if (argc == 1) {
    scan(0);
  } else {
    for (i = 1; i < argc; i++) {
      fd = open(argv[i], O_RDONLY);
      if (fd < 0) {
        fprintf(2, "sixfive: cannot open %s\n", argv[i]);
        continue;
      }
      scan(fd);
      close(fd);
    }
  }
  exit(0);
}
