#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>

using namespace std;

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
}

void sigchildHandler(int sig_num) {
    //waitid for every process - in an unblocking way
    int status;
    int pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    //handle stopped process
    if (WIFSTOPPED(status)) {

    }
        //handle terminated process
    else {
        SmallShell::getInstance().getJobs()->removeJobByPid(pid);
    }
}

