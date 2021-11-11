#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define SIZEOF_BUF  4096

void handle_event(int fd, int wd, char* pathname) {
    char buf[SIZEOF_BUF];
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;
    for(;;) {
        len = read(fd, buf, sizeof(buf));
        if(len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if(len <= 0) {
            break;
        }

        for(ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event*)buf;
            if(event->mask & IN_CREATE) {
                printf("IN_CREATE: ");
            }
        }
        if(wd == event->wd) {
            printf("%s/", pathname);
        }
        if(event->len) {
            printf("%s", event->name);
        }

        if(event->mask & IN_ISDIR) {
            printf(" [directory]\n");
        } else {
            printf(" [file]\n");
        }
    }
}


int main(int argc, char* argv[]) {
    if(argc != 2) {
        perror("argc");
        exit(EXIT_FAILURE);
    }

    char buf;
    int fd, poll_num, wd;
    nfds_t nfds;
    struct pollfd fds[2];

    fd = inotify_init1(IN_NONBLOCK);
    if(fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, argv[1], IN_CREATE);
    if(wd == -1) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    nfds = 2;
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = fd;
    fds[1].events = POLLIN;
    printf("Listening for events.\n");

    while(1) {
        poll_num = poll(fds, nfds, -1);
        if(poll_num == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if(poll_num > 0) {
            if (fds[0].revents & POLLIN) {
                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }
            if (fds[1].revents & POLLIN) {
                handle_event(fd, wd, argv[1]);
            }
        }
    }

    printf("Listening for events stopped.\n");
    close(fd);
    return 0;
}

