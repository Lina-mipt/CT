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

const char *mode(__u16 st) {
  switch (st & S_IFMT) {
  case S_IFBLK:
    return "block device\n";
  case S_IFCHR:
    return "character device\n";
  case S_IFDIR:
    return "directory\n";
  case S_IFIFO:
    return "FIFO/pipe\n";
  case S_IFLNK:
    return "symlink\n";
  case S_IFREG:
    return "regular file\n";
  case S_IFSOCK:
    return "socket\n";
  default:
    return "unknown?\n";
  }
}

static void printf_time(const char *type, struct statx_timestamp *ts) {
  struct tm time;
  time_t time_;
  char buff[128];

  time_ = ts->tv_sec;
  time = *localtime(&time_);
  strftime(buff, 128, "%F %T", &time);
  printf("%s %s.%09u ", type, buff, ts->tv_nsec);
  strftime(buff, 128, "%z", &time);
  printf("%s\n", buff);
}

char *UserName(uid_t uid) {
  struct passwd *name;

  name = getpwuid(uid);
  return (name == NULL) ? NULL : name->pw_name;
}

char *GroupName(uid_t uid) {
  struct group *name;

  name = getgrgid(uid);
  return (name == NULL) ? NULL : name->gr_name;
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

  printf("File size: %u bytes\t", (uint32_t)st.stx_size);
  printf("Blocks allocated: %u\t", (uint32_t)st.stx_blocks);
  printf("Preferred I/O block size: %u bytes\t", (uint32_t)st.stx_blksize);

  printf("%s", mode(st.stx_mode));

  printf("ID of containing device: %u, %u \t",
         (uint32_t)(major(st.stx_dev_major)),
         (uint32_t)minor(st.stx_dev_minor));
  printf("I-node number: %u\t", (uint32_t)st.stx_ino);
  printf("Link count: %u\t\n", (uint32_t)st.stx_nlink);

  printf("Asses:  (0%jo/", (uintmax_t)st.stx_mode & 0777);

  printf((st.stx_mode & S_IFMT) & S_IFDIR ? "d" : "-");

  for (int i = 0; i < 9; i++) {
    putchar(((st.stx_mode & 0777) & (256 >> i)) ? "rwx"[i % 3] : '-');
  }

  printf(")\t");

  printf("Uid: (%u/\t%s)\tGid: (%u/\t%s)\n", (uint32_t)st.stx_uid,
         UserName(st.stx_uid), (uint32_t)st.stx_gid, GroupName(st.stx_gid));

  printf_time("Last file access: ", &st.stx_atime);
  printf_time("Last status change: ", &st.stx_ctime);
  printf_time("Last file modification: ", &st.stx_mtime);
  printf_time("Creation: ", &st.stx_btime);
  exit(EXIT_SUCCESS);
}
