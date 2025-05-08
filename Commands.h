// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <cstring>




using namespace std;
#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
protected:
    const char* cmd_line;
public:
    explicit Command(const char *cmd_line) : cmd_line(cmd_line){};

    virtual ~Command(){
   //    delete cmd_line;
    };

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line): Command(cmd_line){};

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    string original_line;
    explicit ExternalCommand(const char *cmd_line, string original_line): Command(cmd_line),
    original_line(move(original_line)){}

    virtual ~ExternalCommand() {
    }

    void execute() override;

    void executeComplex();
};


class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line) : Command(cmd_line){}

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line): Command(cmd_line){}

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    explicit DiskUsageCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const char *cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:

    explicit ChangePromptCommand(const char *cmd_line): BuiltInCommand(cmd_line){};


    void execute() override;
};


class ChangeDirCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    char ** m_plastPwd;

public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line),m_plastPwd(plastPwd){};
    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    explicit GetCurrDirCommand(const char *cmd_line): BuiltInCommand(cmd_line){};

    void execute() override;
};


class ShowPidCommand : public BuiltInCommand {
public:
    explicit ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
    public:
    QuitCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobs(jobs){};

    virtual ~QuitCommand() {
    }

    void execute() override;
};


class JobsList {
public:
    int max_id = 0;
    class JobEntry {
    public:
        int id;
        int pid;
        string cmd_line;
        bool isStopped;

        JobEntry(int id, int pid, string cmd_line, bool isStopped = false): id(id), pid(pid),
                cmd_line(cmd_line), isStopped(isStopped){};
        ~JobEntry() = default;
    };
    map<int,JobEntry*> job_map;
    map<int, int> pid_to_id_map;
public:
    JobsList() = default;

    ~JobsList() {
        int x;
    }

    void addJob(int pid, const string &cmd_line, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    void removeJobByPid(int jobPid);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed

    JobEntry *getLastStoppedJob();
};

class JobsCommand : public BuiltInCommand {
    JobsList* jobs;
public:
    JobsCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobs(jobs){}

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* m_jobs;
public:
    KillCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),m_jobs(jobs){}

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* m_jobs;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),m_jobs(jobs){};

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand {
public:
    explicit AliasCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char *cmd_line): BuiltInCommand(cmd_line){}

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
    unordered_map<string, string> vars;
public:
    explicit UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;

    void deleteEnviromentVars(unordered_set<string> &unwanted_vars);
};

class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const char *cmd_line): BuiltInCommand(cmd_line){}

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();
    int fg_pid = -1;
    string name = "smash";
    JobsList* jobs = new JobsList;
    unordered_map<string, string> aliases;
    unordered_set<string> saved_words = {"chprompt","quit","alias","showpid","cd","pwd","jobs","fg","kill",
                                         "unalias","unsetenv","watchproc","du","whoami","netinfo"};
    char *plastPwd = nullptr;

    list<string> aliases_by_order;
    unordered_map<string,_List_iterator<string>> aliases_it_map;
public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    const string &getName() const {
        return name;
    }

    void setName(const string &name) {
        SmallShell::name = name;
    }

    void setPlast(char * pwd) {
        plastPwd = pwd;
    }

    JobsList* getJobs(){
        return jobs;
    }

    void setForegroundPid(int pid){
        fg_pid = pid;
    }

    int getForegroundPid() const {
        return fg_pid;
    }


    ~SmallShell();

    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
    bool isSaved(const string &word) const;

    bool isAlias(const string &alias) const;

    void addAlias(string alias, string cmd_line);

    void removeAlias(string alias);

    void printAliases() const;

    bool isRedirection(const char* cmdLine);

    bool isPipe(const char* cmdLine);

    string getRealNamefromAlias(const string &alias);
};

#endif //SMASH_COMMAND_H_
