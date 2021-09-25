#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
//#include <linux/fcntl.h>
#define BUFF_SIZE 128

const char *mode(unsigned st) {
  switch (st & S_IFMT) {
  case S_IFBLK:
    return "block device";
  case S_IFCHR:
    return "character device";
  case S_IFDIR:
    return "directory";
  case S_IFIFO:
    return "FIFO/pipe";
  case S_IFLNK:
    return "symlink";
  case S_IFREG:
    return "regular file";
  case S_IFSOCK:
    return "socket";
  default:
    return "unknown?";
  }
}

static void printf_time(const char *type, struct statx_timestamp *ts) {
  struct tm time;
  time_t time_;
  char buff[BUFF_SIZE];

  time_ = ts->tv_sec;
  time = *localtime_r(&time_, &time);
  strftime(buff, BUFF_SIZE, "%F %T", &time);
  printf("%s %s.%09u ", type, buff, ts->tv_nsec);
  strftime(buff, BUFF_SIZE, "%z", &time);
  printf("%s\n", buff);
}

char *user_name(uid_t uid) {
  struct passwd *info;
  info = getpwuid(uid);

  return (info == NULL) ? NULL : info->pw_name;
}

char *group_name(uid_t uid) {
  struct group *info;
  info = getgrgid(uid);

  return (info == NULL) ? NULL : info->gr_name;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pathname> \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct statx st;

  if (statx(AT_FDCWD, argv[1], AT_STATX_SYNC_AS_STAT, STATX_ALL, &st) == -1) {
    perror("statx");
    exit(EXIT_FAILURE);
  }
  printf("Fail: %s\n", argv[1]);

  printf("File size: %d bytes\tBlocks allocated: %d\tPreferred I/O block size: "
         "%d bytes\t%s\n",
         (unsigned)st.stx_size, (unsigned)st.stx_blocks,
         (unsigned)st.stx_blksize, mode(st.stx_mode));

  printf("ID of containing device: %d, %d \tI-node number: %d\tLink count: %d\t\n",
         (unsigned)major(st.stx_dev_major), (unsigned)minor(st.stx_dev_minor),
         (unsigned)st.stx_ino, (unsigned)st.stx_nlink);

  printf("Acces:  (0%jo/", (uintmax_t)st.stx_mode & 0777);
  printf((st.stx_mode & S_IFMT) & S_IFDIR ? "d" : "-");

  for (int i = 0; i < 9; i++) {
    putchar(((st.stx_mode & 0777) & (1 << (8 - i))) ? "rwx"[i % 3] : '-');
  }

  printf(")\t");
  printf("Uid: (%d/\t%s)\tGid: (%d/\t%s)\n", (unsigned)st.stx_uid,
         user_name(st.stx_uid), (unsigned)st.stx_gid, group_name(st.stx_gid));

  printf_time("Last file access: ", &st.stx_atime);
  printf_time("Last status change: ", &st.stx_ctime);
  printf_time("Last file modification: ", &st.stx_mtime);
  printf_time("Creation: ", &st.stx_btime);

  exit(EXIT_SUCCESS);
}
