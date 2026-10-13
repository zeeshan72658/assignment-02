#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define PATHSZ 512

static char *target;
static char **command;
static int command_argc;

static int
matches_name(char *path)
{
  int end, start, i, target_len;

  end = strlen(path);
  while (end > 1 && path[end - 1] == '/')
    end--;
  start = end;
  while (start > 0 && path[start - 1] != '/')
    start--;
  target_len = strlen(target);
  if (end - start != target_len)
    return 0;
  for (i = 0; i < target_len; i++) {
    if (path[start + i] != target[i])
      return 0;
  }
  return 1;
}

static void
report(char *path)
{
  char *args[MAXARG];
  int i, pid;

  if (command == 0) {
    printf("%s\n", path);
    return;
  }

  for (i = 0; i < command_argc; i++)
    args[i] = command[i];
  args[command_argc] = path;
  args[command_argc + 1] = 0;

  pid = fork();
  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }
  if (pid == 0) {
    exec(command[0], args);
    fprintf(2, "find: cannot execute %s\n", command[0]);
    exit(1);
  }
  wait(0);
}

static void
walk(char *path)
{
  char buf[PATHSZ], name[DIRSIZ + 1];
  char *p;
  int fd;
  struct dirent de;
  struct stat st;

  if (stat(path, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    return;
  }

  if (matches_name(path))
    report(path);

  if (st.type != T_DIR)
    return;
  fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  if (p > buf && p[-1] != '/')
    *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = '\0';
    walk(buf);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "Usage: find path name [-exec command [args...]]\n");
    exit(1);
  }
  target = argv[2];
  if (argc > 3) {
    if (strcmp(argv[3], "-exec") != 0 || argc < 5) {
      fprintf(2, "Usage: find path name [-exec command [args...]]\n");
      exit(1);
    }
    command_argc = argc - 4;
    if (command_argc + 2 > MAXARG) {
      fprintf(2, "find: too many command arguments\n");
      exit(1);
    }
    command = &argv[4];
  }

  walk(argv[1]);
  exit(0);
}
