#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>

#define LOG(fmt, ...) fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)
#define ERR(fmt) fprintf(stderr, "line%d: " fmt "   %s\n", __LINE__, strerror(errno))

int isINT(char *);
void ltoa(uid_t uid, char *str);

int main(int argc, char *argv[])
{
    DIR *dirp;
    dirp = opendir("/proc");
    if (!dirp)
        ERR("opendir");
    struct dirent *direntp;

    uid_t uid = getuid();
    char uids[100];
    ltoa(uid, uids);
    fprintf(stderr, "the current user's id is %s\n", uids);
    fprintf(stderr, "the follow id belongs to user \n");

    while((direntp = readdir(dirp)) != NULL)
    {
        //printf("%s\n", direntp->d_name);
        char fileName[100] = "/proc/";
        if (isINT(direntp->d_name))
            {strcat(fileName, direntp->d_name); strcat(fileName, "/status"); }
        else
            continue;

        //LOG("%s", fileName);
        char line[1000];
        FILE* file = fopen(fileName,"r");
       // ERR("open");
        while(fgets(line, 1000, file) != NULL)
        {
            if ((strstr(line, uids)) != NULL && (strstr(line, "Uid")) != NULL)
                fprintf(stderr, "%s\n", direntp->d_name);
        }
        
        fflush(NULL);

    }
    fflush(NULL);
    _exit(0);
}
int isINT(char *str)
{
    char *tempStr = str;
    while (*tempStr != '\0')
    {
        if (*tempStr >= '0' && *tempStr <= '9')
            tempStr++;
        else
            return 0;
    }
    return 1;
}
void ltoa(uid_t uid, char *str)
{
    char temp[100];
    char *temps = str;
    char *tempStr = temp;
    *tempStr = '\0';
    tempStr++;
    while (uid > 0)
    {
        int c = uid % 10 + '0';
        uid = uid / 10;
        *tempStr = c;
        tempStr++;
    }
    tempStr--;
    while(*tempStr)
        {*temps++ = *tempStr--; }
    //LOG("%s", str);
    *temps = '\0';
    return;
    
}
