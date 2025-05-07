#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>

using namespace std;

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    int fg_pid = SmallShell::getInstance().getForegroundPid();

    //if fg process other than shell exists, the fg_pid it will not be -1
    if ( fg_pid != -1){
        kill(fg_pid, SIGKILL);
      cout <<  "smash: process " << fg_pid << " was killed" << endl;
    }
}

void sigchildHandler(int sig_num) {
    //waitid for every process - in an unblocking way
    int status;
    int pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    //handle stopped process
    if(WIFSTOPPED(status)){

    }
    //handle terminated process
    else { // add if is direct son of shell pid
        SmallShell::getInstance().getJobs()->removeJobByPid(pid);
    }
}

