#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#define LOG(fmt, ...) fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)
#define ERR(fmt) fprintf(stderr, "line%d: " fmt "   %s\n", __LINE__, strerror(errno))

int _dup(int);
int _dup2(int, int);
int
main(int argc, char *argv[])
{
    int fd1  = _dup(2);
    write(fd1, "test _dup\n", 10);
    int fd2 = _dup2(2, 5);
    write(fd2, "test _dup2\n", 11);
    int fd3 = _dup2(5, 5);
    write(fd3, "test _dup2\n", 11);
    _exit(0);
}
int _dup(int oldfd)
{
    return fcntl(oldfd, F_DUPFD, 0);
}
int _dup2(int oldfd, int newfd){
    int ret;
    ret = fcntl(oldfd, F_GETFL);
    if (ret == -1)
        return -1;
    if (oldfd == newfd)
        return oldfd;
    ret = fcntl(newfd, F_GETFL);
    if (ret)
        close(newfd);
    return fcntl(oldfd, F_DUPFD, newfd);
    
}
