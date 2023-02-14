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


int
main(int argc, char *argv[])
{
    int fd;
    int64_t off;

    if (argc == 4)
        fd = open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    else
        fd = open(argv[1], O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);

    if (fd == -1)
        ERR("open");

    off = atoll(argv[2]);
    if (argc == 4 )
    {
        while (off--)
        {
            if (lseek(fd, 0, SEEK_END) == -1)
                ERR("lseek");

            if (write(fd, "t", 1) == -1)
                ERR("write");
        }
    }
    else{
         while (off--)
        {
            if (write(fd, "t", 1) == -1)
                ERR("write");
        }
    }
    _exit(0);
}
