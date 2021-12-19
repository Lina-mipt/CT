#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(void)
{
    int res = 0;
    const char *queue_name = "/test-mqueue";
    mqd_t queue_fd = mq_open(queue_name, O_RDWR | O_NONBLOCK | O_CREAT, 0600, NULL);
    if (queue_fd == (mqd_t)-1)
    {
        perror("mq_open");
        res = 1;
        return res;
    }

    if (mq_send(queue_fd, "hello", strlen("hello"), 0) < 0)
    {
        perror("mq_send");
        res = 1;
        if (mq_close(queue_fd) == -1)
            res = 2;
        if (mq_unlink(queue_name) == -1)
            res = 3;
        return res;
    }

    struct mq_attr m_info;
    if (mq_getattr(queue_fd, &m_info) == -1)
        res = 4;
    else
    {
        printf("Flags: %ld\n", m_info.mq_flags);
        printf("Max. # message on queue: %ld\n", m_info.mq_maxmsg);
        printf("Max. message size (bytes): %ld\n", m_info.mq_msgsize);
        printf("# of messages currently in queue: %ld\n", m_info.mq_curmsgs);
    }

    if (mq_close(queue_fd) == -1)
    {
        res = 5;
        perror("mq_close");
    }
    if (mq_unlink(queue_name) == -1)
    {
        res = 6;
        perror("mq_link");
    }
    return res;
}
