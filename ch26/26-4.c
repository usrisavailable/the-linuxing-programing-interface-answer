#include "debug.h"

#define CMD_LINE 100
#define SYNC_SIG SIGUSR1

static void sighandler(int sig)
{}

int main(int argc, char *argv[])
{
    //send block catch (process sync via signal)
    char cmd[CMD_LINE];
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;
    setbuf(stdout, NULL);
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SYNC_SIG);
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask");
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sighandler;
    if (sigaction(SYNC_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    setbuf(stdout, NULL);

    LOG("parent pid=%ld", getpid());

    switch(childPid = fork()) {
        case -1:
            errExit("fork");
        case 0:
            sigemptyset(&emptyMask);
            LOG("child wait for signal from %ld", (long) getppid());
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR) //interuted system call
                errExit("sigsuspend");
            LOG("child %ld exited", (long) getpid());
            _exit(0);
        default:
            snprintf(cmd, CMD_LINE, "ps | grep %s", basename(argv[0]));
            cmd[CMD_LINE - 1] = '\0';
            system(cmd);
            if (kill(childPid, SYNC_SIG) == -1)
                ERR("kill");
            sleep(3);
            if (kill(childPid, SIGKILL) == -1)
                ERR("kill");
            LOG("after sending SIGKILL to zombie %ld ", (long)childPid);
            system(cmd);
            exit(0);
            
    }
    return 0;
}
