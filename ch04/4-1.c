#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#define debug(STR, ...) fprintf(stderr, "line%d: " STR "\n", __LINE__, __VA_ARGS__)
#define MAXLINE 1000

int main(int argc, char *argv[])
{
    char *s;
    size_t n;
    FILE *file;
    size_t num;
    //这要比直接去解析要简单
    if (argc == 2)
        file = fopen(argv[1], "w");
    if (argc == 3)
        file = fopen(argv[2], "a");

    while((num = getline(&s, &n, stdin)) != -1)
    {
        printf("%s", s);
        debug("%ld", num);
        if(!fwrite(s, num, 1, file))
            debug("%s", strerror(errno));
    }
    return 0;
}

