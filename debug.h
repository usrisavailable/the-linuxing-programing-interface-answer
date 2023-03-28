#pragma(once)

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h> //definition of  uint64_t
#include <ctype.h>
#include <libgen.h> // for basename() declaration


/* 调试有关的函数在这里处理 */

#define LOG(fmt, ...) fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)
#define ERR(fmt) fprintf(stderr, "line%d: " fmt "   %s\n", __LINE__, strerror(errno))

/* 常用基本处理函数在这里处理 */
void errExit(char *str)
{
    fprintf(stderr, "%s %s\n", str, strerror(errno));
    exit(0);
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

/* 信号有关的函数在这里处理 */
void
printSigset(FILE *of, const char *prefix, const sigset_t *sigset)
{
    int sig, cnt;

    cnt = 0;
    for (sig = 1; sig < NSIG; sig++)
    {
        if (sigismember(sigset,sig)){
            cnt ++;
            fprintf(of, "%s%d (%s)\n", prefix, sig, strsignal(sig));
        }
    }
    if (cnt == 0)
        fprintf(of, "%s<empty signal set>\n", prefix);

    return ;
}

int
printSigMask(FILE *of, const char *msg)
{
    sigset_t currMask;
    if (msg != NULL)
        fprintf(of, "%s", msg);
    
    if (sigprocmask(SIG_BLOCK, NULL, &currMask) == -1)
        return -1;

    printSigset(of, "\t\t", &currMask);
    return 0;
}
int 
printPendingSigs(FILE *of, const char *msg)
{
    sigset_t pendingSigs;

    if (msg != NULL)
        fprintf(of, "%s", msg);

    if (sigpending(&pendingSigs) == -1)
        return -1;
    printSigset(of, "\t\t", &pendingSigs);
}

void
printWaitStatus(const char* msg, int status)
{
    if (msg != NULL)
        LOG("%s", msg);
    if (WIFEXITED(status)){
        LOG("child exited, status=%d", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)){
        LOG("child killed by signal %d (%s)",
            WTERMSIG(status), strsignal(WTERMSIG(status)));
    } else if (WIFSTOPPED(status)) {
        LOG("chiled stopped by signal %d (%s)",
            WSTOPSIG(status), strsignal(WSTOPSIG(status)));
    } else if (WIFCONTINUED(status)) {
        LOG("child continued", "%s");
    } else {
        LOG("what happened to this child? (status=%x)",
            (unsigned int) status);
    }
}

char *currTime()
{
    long now = time(NULL);
    char *str = ctime(&now);
    char *start;
    char *temp = str;
    int count = 0;
    //LOG("%s", str);
    while(*temp)
    {
        if (isspace(*temp))
        {
            count++;
            if (count == 3)
                start = temp + 1;
            if (count == 4)
                {*temp = '\0'; break;}
        }
        temp++;
    }
    return start;
}
