#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <unordered_map>
#include <regex>
#include "Commands.h"
#include <fcntl.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>


using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {

// TODO: add your implementation
}

SmallShell::~SmallShell() {
    delete jobs;
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/


Command *SmallShell::CreateCommand(const char *cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string original_line = cmd_s;
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  //handle case where first word is alias

  if(SmallShell::getInstance().isAlias(firstWord)){
      //get real name of alias
      string real_name = SmallShell::getInstance().getRealNamefromAlias(firstWord);

      //replace alias with real name
      string line_without_first_word = cmd_s.substr(firstWord.length(),cmd_s.length());
      char *real_line = new char(cmd_s.length() + (real_name.length()-firstWord.length()));
      string tmp = real_name + line_without_first_word;
      strcpy(real_line,tmp.c_str());
      cmd_line = real_line;
      firstWord = real_name.substr(0,real_name.find_first_of(" \n"));
  }

  if(SmallShell::isRedirection(cmd_line)){
        return new RedirectionCommand(cmd_line);
  }
  if(SmallShell::isPipe(cmd_line)){
        return new PipeCommand(cmd_line);
  }
  if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0){
    return new GetCurrDirCommand(cmd_line);
  }

    if(firstWord.compare("du") == 0 || firstWord.compare("du&") == 0){
        return new DiskUsageCommand(cmd_line);
    }

  if (firstWord.compare("whoami") == 0 || firstWord.compare("whoami&") == 0){
        return new WhoAmICommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0 ) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0 ) {
      return new ChangeDirCommand(cmd_line, &getInstance().plastPwd);
  }
  else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0){
      return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0){
     return new ForegroundCommand(cmd_line,getInstance().jobs);
 }

  else if (firstWord.compare("kill") == 0){
    return new KillCommand(cmd_line,getInstance().jobs);
 }

  else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0){
      return new JobsCommand(cmd_line, this->jobs);
  }
  else if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0){
      return new QuitCommand(cmd_line, this->jobs);
  }
  else if (firstWord.compare("alias") == 0){
      return new AliasCommand(cmd_line);
  }

  else if (firstWord.compare("unsetenv") == 0 || firstWord.compare("unsetenv&") == 0){
      return new UnSetEnvCommand(cmd_line);
  }

  else if (firstWord.compare("unalias") == 0){
      return new UnAliasCommand(cmd_line);
  }

  else if (firstWord.compare("watchproc") == 0){
      return new WatchProcCommand(cmd_line);
  }

  return new ExternalCommand(cmd_line, original_line);
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here

     Command* cmd = CreateCommand(cmd_line);
     if(cmd) {
         cmd->execute();
     }
     // delete cmd; // - possibly too much memory will be allocated
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

bool SmallShell::isSaved(const string &word) const {
    if(this->saved_words.find(word) == saved_words.end()) {
        return false;
    }
    return true;
}

bool SmallShell::isAlias(const string &alias) const {
    if(this->aliases.find(alias) == this->aliases.end()) {
        return false;
    }
    return true;
}

void SmallShell::addAlias(string alias, string cmd_line) {
    this->aliases[alias] = cmd_line;

    //prepare to print by order
    auto it = aliases_by_order.insert(aliases_by_order.end(), alias);
    
    //prepare for fast deletion
    aliases_it_map[alias] = it;
}

void SmallShell::removeAlias(string alias) {
    this->aliases.erase(alias);
    auto it = aliases_it_map[alias];
    aliases_by_order.erase(it);
    aliases_it_map.erase(alias);
}

void SmallShell::printAliases() const {
    //print by order
    for(const string & alias : aliases_by_order){
        auto it = aliases.find(alias);
        cout << alias << "='" << it->second <<"'" << endl;
    }

    aliases_by_order.begin();
}

//size is the size of the new desired char* to hold the real name
string SmallShell::getRealNamefromAlias(const string &alias) {
    if(!isAlias(alias)) {
        cerr << "tried to access a non-exiting alias" << endl;
        return "";
    }

    return this->aliases[alias];
    /*
    string tmp =  this->aliases[alias];
    char* real_name = new char(size);
    for(int i = 0; i < tmp.length(); i++){
        real_name[i] = tmp[i];
    }
    return real_name;
     */
}

// TODO: temporary?
std::vector<std::string> _parse(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> result;
    std::string word;
    while (iss >> word) {
        result.push_back(word);
    }
    return result;
}


void UnAliasCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string tmp = cmd_s.substr(7,cmd_s.length()); //unalias length is 7
    //TODO: replace the _parse method to _parseCommandLine method provided
    std::vector<std::string> args = _parse(tmp);

    if(args.size() == 0){
        cerr << "smash error: unalias: not enough arguments" << endl;
        return;
    }

    for(auto& alias : args){
        if(SmallShell::getInstance().isAlias(alias)){
            SmallShell::getInstance().removeAlias(alias);
        }
        else{
            cerr << "smash error: unalias: " << alias << " alias does not exist"
                    << endl;
            return;
        }
    }
}

struct Stat {
    pid_t               pid{};
    std::string         comm;
    char                state{};
    pid_t               ppid{};
    pid_t               pgrp{};
    unsigned long       utime{};
    unsigned long       stime{};
    long                rss{};
    unsigned long long  starttime{};
};

void WatchProcCommand::execute() {
    string cmd_line_str = _trim(string(this->cmd_line));
    string pid_str = _trim(cmd_line_str.substr(9));

    if (pid_str.empty()) {
        cerr << "smash error: watchproc: invalid arguments" << endl;
        return;
    }

    for (char c: pid_str) {
        if (!isdigit(c)) {
            cerr << "smash error: watchproc: invalid arguments" << endl;
            return;
        }
    }

    int pid = stoi(pid_str);

    string stat_path = "/proc/" + pid_str + "/stat";
    int stat_fd = open(stat_path.c_str(), O_RDONLY);
    if (stat_fd == -1) {
        cerr << "smash error: watchproc: pid " << pid << " does not exist" << endl;
        return;
    }

    const int BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    ssize_t bytes_read = read(stat_fd, buffer, BUFFER_SIZE - 1);
    close(stat_fd);

    if (bytes_read <= 0) {
        cerr << "smash error: watchproc: can't read process stats" << endl;
        return;
    }

    buffer[bytes_read] = '\0';
    string stat_line(buffer);

    size_t pos = stat_line.find(')');
    if (pos == string::npos) {
        cerr << "smash error: watchproc: pid " << pid << " does not exist" << endl;
        return;
    }

    string remainder = stat_line.substr(pos + 2);
    istringstream stat_stream(remainder);
    string value;

    stat_stream >> value;

    for (int i = 0; i < 10; i++) {
        if (!(stat_stream >> value)) {
            cerr << "smash error: watchproc: can't read process stats" << endl;
            return;
        }
    }
    unsigned long utime = 0, stime = 0, starttime = 0;
    if (!(stat_stream >> utime >> stime)) {
        cerr << "smash error: watchproc: can't read process CPU stats" << endl;
        return;
    }

    for (int i = 0; i < 7; i++) {
        if (!(stat_stream >> value)) {
            cerr << "smash error: watchproc: can't read process stats" << endl;
            return;
        }
    }

    if (!(stat_stream >> starttime)) {
        cerr << "smash error: watchproc: can't read process start time" << endl;
        return;
    }

    int uptime_fd = open("/proc/uptime", O_RDONLY);
    if (uptime_fd == -1) {
        cerr << "smash error: watchproc: can't read system uptime" << endl;
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(uptime_fd, buffer, BUFFER_SIZE - 1);
    close(uptime_fd);

    if (bytes_read <= 0) {
        cerr << "smash error: watchproc: can't read system uptime" << endl;
        return;
    }

    buffer[bytes_read] = '\0';
    istringstream uptime_stream(buffer);
    double system_uptime = 0.0;
    uptime_stream >> system_uptime;

    long clock_ticks = sysconf(_SC_CLK_TCK);
    double process_runtime = system_uptime - (starttime / static_cast<double>(clock_ticks));
    double cpu_time_seconds = (utime + stime) / static_cast<double>(clock_ticks);
    double cpu_percent = 0.0;

    if (process_runtime > 0) {
        cpu_percent = 100.0 * (cpu_time_seconds / process_runtime);
    }

    string status_path = "/proc/" + pid_str + "/status";
    int status_fd = open(status_path.c_str(), O_RDONLY);

    if (status_fd == -1) {
        cerr << "smash error: watchproc: pid " << pid << " does not exist" << endl;
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(status_fd, buffer, BUFFER_SIZE - 1);
    close(status_fd);

    if (bytes_read <= 0) {
        cerr << "smash error: watchproc: can't read process memory stats" << endl;
        return;
    }

    buffer[bytes_read] = '\0';
    string status_content(buffer);

    // Find VmRSS line
    double memory_kb = 0.0;
    bool found_memory = false;
    istringstream status_stream(status_content);
    string line;

    while (getline(status_stream, line)) {
        if (line.find("VmRSS:") == 0) {
            istringstream line_stream(line);
            string label;
            line_stream >> label >> memory_kb;
            found_memory = true;
            break;
        }
    }

    if (!found_memory) {
        memory_kb = 0.0;
    }

    double memory_mb = memory_kb / 1024.0;

    cout << fixed << setprecision(1);
    cout << "PID: " << pid << " | CPU Usage: " << cpu_percent
         << "% | Memory Usage: " << memory_mb << " MB" << endl;
}


void KillCommand::execute() {
    std::string cmd_line_str = _trim(cmd_line);

    //TODO: replace the _parse method to _parseCommandLine method provided
    std::vector<std::string> args = _parse(cmd_line_str);

    if (args.size() != 3 || args[1][0] != '-') {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    int sig;
    int job_id;
    try {
        sig = std::stoi(args[1].substr(1));
        job_id = std::stoi(args[2]);
    } catch (...) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    auto it = SmallShell::getInstance().getJobs()->job_map.find(job_id);
    if (it == SmallShell::getInstance().getJobs()->job_map.end()) {
        std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << std::endl;
        return;
    }
    auto job = it->second;
    int pid = job->pid;
    if (kill(pid, sig) == -1) {
        perror("smash error: kill failed");
        return;
    }
    std::cout << "signal number " << sig << " was sent to pid " << pid << std::endl;
}

void ForegroundCommand::execute() {
    std::string cmd_line_str = _trim(cmd_line);

    //TODO: replace the _parse method to _parseCommandLine method provided
    std::vector<std::string> args = _parse(cmd_line_str);

    if (args.size() > 2) {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    int job_id;



    if(args.size() == 1) {
        if (m_jobs->job_map.empty()) {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        } else {
            job_id = m_jobs->max_id;
        }
    } else {
        try{
            job_id = stoi(args[1]);
        }
        catch (...){
            std::cerr << "smash error: fg: invalid arguments" << std::endl;
            return;
        }
    }
    auto it = m_jobs->job_map.find(job_id);
    if (it == m_jobs->job_map.end()) {
        std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << std::endl;
        return;
    }
    auto job = it->second;
    int pid = job->pid;
    std::cout << job->cmd_line << " " << pid << std::endl;
    SmallShell::getInstance().setForegroundPid(pid);
    int status;
    waitpid(pid, &status, WUNTRACED);

    SmallShell::getInstance().setForegroundPid(-1);

    //removes from joblist
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        m_jobs->removeJobById(job_id);
        //for cases user stopped it
    } else if (WIFSTOPPED(status)) {
        job->isStopped = true;
    }
}

void ChangePromptCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string tmp = cmd_s.substr(8,cmd_s.length()); //chprompt length is 8
    cmd_s = _trim(tmp);
    string secondWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if(secondWord[secondWord.length()-1] == '&'){
        secondWord = secondWord.substr(0, secondWord.length() - 1);
    }
    //Replacing shell name to second word if it exists
    if(secondWord == "" || secondWord == " "|| secondWord == "&"){
        SmallShell::getInstance().setName("smash");
    } else {
    SmallShell::getInstance().setName(secondWord);
    }
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

void ChangeDirCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string tmp = cmd_s.substr(2); // cd length is 2
    cmd_s = _trim(tmp);

    const int LENGTH = 1024;
    char *old = new char[LENGTH];
    if (getcwd(old, LENGTH) == nullptr) {
        perror("smash error: getcwd");
        delete[] old;
        return;
    }

    string secondWord = cmd_s;
    if (cmd_s.find(' ') != std::string::npos) {
        secondWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
        if (cmd_s.substr(secondWord.length()).find_first_not_of(" \t\n") != std::string::npos) {
            cout << "smash error: cd: too many arguments" << endl;
            delete[] old;
            return;
        }
    }

    if (cmd_s == "-") {
        if (!m_plastPwd || !(*m_plastPwd)) {
            cout << "smash error: cd: OLDPWD not set" << endl;
            delete[] old;
            return;
        }

        if (chdir(*m_plastPwd) == -1) {
            perror("smash error: chdir");
            delete[] old;
            return;
        }

        SmallShell::getInstance().setPlast(strdup(old));
        delete[] old;
        return;
    } else if (!cmd_s.empty()) {

        const char *dest = secondWord.c_str();
        if (chdir(dest) == -1) {
            std::string altPath = std::string(dest) + "/";
            if (chdir(altPath.c_str()) == -1) {
                perror("smash error: chdir");
                delete[] old;
                return;
            }
        }
    } else {
        delete[] old;
        return;
    }

    SmallShell::getInstance().setPlast(strdup(old));
    delete[] old;
}

void GetCurrDirCommand::execute() {
    int size = 100;
    char *pathName = new char(size);
    while(getcwd(pathName,size) == nullptr){
        char* tmp = pathName;
        size *= 2;
        pathName = new char (size);
        delete tmp;
    }
    cout << pathName << endl;
}

void JobsCommand::execute() {
    if(jobs) {
        this->jobs->printJobsList();
    }
}

void JobsList::printJobsList() {
    for(auto pair : this->job_map){
        cout << "[" << pair.first <<"] " << pair.second->cmd_line << endl;

    }
}

void JobsList::killAllJobs() {
    for (const auto& pair : job_map){
        kill(pair.second->pid, SIGKILL);
        delete pair.second;
    }
    job_map.clear();
}

void JobsList::addJob(int pid, const string &cmd_line, bool isStopped) {
    int id = max_id + 1;
    JobEntry* job = new JobEntry(id, pid, cmd_line, isStopped);
    job_map[id] = job;
    pid_to_id_map[pid] = id;
    max_id++;

}

void JobsList::removeJobByPid(int jobPid) {
    auto it = pid_to_id_map.find(jobPid);
    if(it != pid_to_id_map.end()) {
        int id = pid_to_id_map[jobPid];
        removeJobById(id);
    }
}

void JobsList::removeJobById(int jobId) {
    auto it1 = this->job_map.find(jobId);
    int pid;
    if (it1 != job_map.end()) {
        pid = it1->second->pid;
        delete it1->second;
        job_map.erase(it1);
    } else {
        return;
    }
    auto it2 = this->pid_to_id_map.find(pid);
    if(it2 != pid_to_id_map.end()){
        pid_to_id_map.erase(it2);
    }
    if(max_id == jobId){
        auto it3 = job_map.rbegin();
        if(it3 != job_map.rend()) {
            max_id = it3->first;
        } else {
            max_id = 0;
        }
    }
}


void QuitCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string tmp = cmd_s.substr(4,cmd_s.length()); //quit length is 4
    cmd_s = _trim(tmp);
    string secondWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    //displaying jobs by pid and original cmd line and then killing them
    if(secondWord == "kill" || secondWord == "kill&"){
        int size = this->jobs->job_map.size();
        cout << "sending SIGKILL to " << size << " jobs" << endl;
        for(auto pair : this->jobs->job_map){
            cout << pair.second->pid <<": " << pair.second->cmd_line << endl;
        }
        this->jobs->killAllJobs();
    }

    //exiting from smash in either case
    exit(0);
}

void AliasCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string tmp = cmd_s.substr(5,cmd_s.length()); //alias length is 5

    if(tmp.find_first_not_of(' ') == string::npos){
        SmallShell::getInstance().printAliases();
        return;
    }
    regex reg ("^alias [a-zA-Z0-9_]+='[^']*'$");

    //if syntax is incorrect return appropriate error
    if(!regex_match(cmd_s,reg)){
       cerr << "smash error: alias: invalid alias format" << endl;
       return;
    }

    //take second word, check if saved or already an alias - if so, return appropriate error
    cmd_s = _trim(tmp);
    string secondWord = cmd_s.substr(0, cmd_s.find_first_of("='"));
    if(SmallShell::getInstance().isSaved(secondWord) || SmallShell::getInstance().isAlias(secondWord)){
        cerr <<"smash error: alias: " << secondWord <<" already exists or is a reserved command" << endl;
        return;
    }

    //add alias to aliases map
    string real_name = cmd_s.substr(cmd_s.find_first_of('\'') + 1,cmd_s.length());
    real_name = real_name.substr(0, real_name.find_first_of('\''));
    SmallShell::getInstance().addAlias(secondWord, real_name);

}



UnSetEnvCommand::UnSetEnvCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    int pid = getpid();
    string environ_path = "/proc/" + to_string(pid) + "/environ";
    int fd = open(environ_path.c_str(),O_RDONLY);
    if (fd == -1){
        cerr << "unable to open environ file" << endl;
    }
    char buffer[1001];
    ssize_t bytes_read = read(fd,buffer,1000);
    char* current_char;
    bool equal_sign_detected = false;
    string new_variable, new_value;

    while(bytes_read > 0){
        buffer[bytes_read] = '\0';
        current_char = buffer;

        //keep looping as long as there are chars, if current char == EOF then buffer is finished
        while(*current_char != EOF) {

            //if line has ended start looking for a new variable name
            if(*current_char == '\0'){
                equal_sign_detected = false;
                vars[new_variable] = new_value;
                new_variable.clear();
                new_value.clear();
            }
                // if encountered equal sign '=', insert new variable name to set
                // start looking for new name after line is finished
            else if(*current_char == '='){
                equal_sign_detected = true;
                new_value += '=';
            }
                //if equal sign has yet been detected add chars to var name
            else if(!equal_sign_detected){
                new_variable += *current_char;
            }
                //if equal sign has been detected add chars to value
            else {
                new_value += *current_char;
            }
            current_char++;
        }
        //read next block if necessary
        bytes_read = read(fd, buffer, 1000);
    }
}



void UnSetEnvCommand::execute() {

    //for every argument validate that it is an environment variable and delete it , stop at error
    string cmd_s = _trim((string)cmd_line);
    cmd_s = cmd_s.substr(8, cmd_s.length());
    unordered_set<string> unwanted_vars;
    cmd_s = _trim(cmd_s);
    if(cmd_s == ""){
        cerr << "smash error: unsetenv: not enough arguments" << endl;
        return;
    }
    while (true){
        cmd_s = _trim(cmd_s);
        int arg_idx = cmd_s.find_first_of(" \n");
        string unwanted_var = cmd_s.substr(0, arg_idx);
        if (vars.find(unwanted_var) == vars.end()){
            deleteEnviromentVars(unwanted_vars);
            cerr <<  "smash error: unsetenv: "<< unwanted_var << " does not exist" << endl;
            return;
        }
        unwanted_vars.insert(unwanted_var);
        if (arg_idx == string :: npos){
            break;
        }
        cmd_s = cmd_s.substr(arg_idx, cmd_s.length());
    }
    deleteEnviromentVars(unwanted_vars);
}

void UnSetEnvCommand::deleteEnviromentVars(unordered_set<string> &unwanted_vars) {

    // mark the index of variables in environ that are to be deleted
    int environ_length = 0;
    vector<int> del_indices;
    for (int i = 0; environ[i] != nullptr; i++){
        string enviro_var = environ[i];
        enviro_var = enviro_var.substr(0, enviro_var.find_first_of('='));
        if (unwanted_vars.find(enviro_var) != unwanted_vars.end()){
            del_indices.push_back(i);
        }
        environ_length++;
    }

    //delete the unwanted variables from environ
    int end = environ_length - 1;
    for (int idx : del_indices){
        swap(environ[idx], environ[end]);
        environ[end] = nullptr;
        end--;
    }
}



enum CommandState {BG, FG};
void ExternalCommand::execute() {
    CommandState cmd_state = FG;

    //preparing the argument vector
    char* argv[23];
    string cmd_s = _trim(string(this->cmd_line));
    int i = 0, arg_idx = cmd_s.find_first_of(" \n");

    //handle complex external commands

    if(cmd_s.find_first_of('*') != string::npos || cmd_s.find_first_of('?') != string::npos){
        executeComplex();
        return;
    }

    //fishing the arguments from the commmand line
    while(true){
        string arg = cmd_s.substr(0, arg_idx);
        argv[i] = new char(arg.length() + 1);
        strcpy(argv[i], arg.c_str());
        if (arg_idx == -1){
            i++;
            break;
        }
        cmd_s = cmd_s.substr(arg_idx,cmd_s.length());
        cmd_s = _trim(cmd_s);
        arg_idx = cmd_s.find_first_of(" \n");
        i++;
    }
    argv[i] = nullptr;

    //check if & is the last char in the last argument
    int j = 0;
    while(argv[i-1][j + 1]){
        j++;
    }
    if(argv[i-1][j] == '&'){
        cmd_state = BG; //make sure command runs in background
        if(j == 0){
            argv[i-1] = nullptr;
        } else {
            argv[i - 1][j] = '\0';
        }
    }

    //fork a new process, if command is in background add it to jobList
    int pid = fork();
    if(pid == 0 ){
        setpgrp();
        execvp(argv[0], argv);
    } else if (cmd_state == FG){
        SmallShell::getInstance().setForegroundPid(pid);
        wait(NULL);
        SmallShell::getInstance().setForegroundPid(-1);
    } else if (cmd_state == BG){
        SmallShell::getInstance().getJobs()->addJob(pid, original_line);
    }
}

void ExternalCommand::executeComplex() {
    string complex_path = "/bin/bash";
    string complex_flag = "-c";
    CommandState cmd_state = FG;

    //make new argv
    char* argv[4];
    argv[0] = new char(complex_path.length() + 1);
    strcpy(argv[0], complex_path.c_str());
    argv[1] = new char(complex_flag.length() + 1);
    strcpy(argv[1], complex_flag.c_str());
    string orig_line = _trim(this->original_line);
    argv[2] = new char(orig_line.length() + 1);
    strcpy(argv[2], orig_line.c_str());
    argv[3] = nullptr;

    //check if last char is ampersand, if so prepare to run in background
    if(orig_line[orig_line.length() - 1] == '&'){
        cmd_state = BG;
        argv[2][orig_line.length() - 1] = '\0';
    }

    //fork the process
    int pid = fork();
    if (pid == 0){
        setpgrp();
        execv(complex_path.c_str(), argv); // complex_path = "/bin/bash
    } else if (cmd_state == FG) {
        SmallShell::getInstance().setForegroundPid(pid);
        wait(NULL);
        SmallShell::getInstance().setForegroundPid(-1);
    } //add to job list if necessary
    else if (cmd_state == BG){
        SmallShell::getInstance().getJobs()->addJob(pid, original_line);
    }
}


bool SmallShell::isRedirection(const char *cmdLine) {
    string cmd_s = _trim(cmdLine);
    if(cmd_s.find_first_of(">>") != string::npos || cmd_s.find_first_of('>') != string::npos){
        return true;
    }
    return false;

//    int i = 0;
//    while (cmdLine[i] != '\0') {
//        if (cmdLine[i] == '>') {
//            int j = i;
//            while (cmdLine[j] == '>') {
//                j++;
//            }
//            return j - i;
//        }
//        i++;
//    }
//    return (i == 1 || i == 2);
}

bool SmallShell::isPipe(const char *cmdLine) {
    string cmd = cmdLine;
    for (auto c: cmd) {
        if (c == '|') {
            return true;
        }
    }
    return false;
}



void RedirectionCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string op;
    size_t pos = cmd_s.find(">>");
    if (pos != string::npos) {
        op = ">>";
    } else {
        pos = cmd_s.find(">");
        if (pos != string::npos) {
            op = ">";
        } else {
            return;
        }
    }

    string lhs = _trim(cmd_s.substr(0, pos));
    string rhs = _trim(cmd_s.substr(pos + op.length()));

    int fd;

    if (op == ">>") {
        fd = open(rhs.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else {
        fd = open(rhs.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    int shellFd = dup(STDOUT_FILENO);
    if (shellFd == -1) {
        perror("smash error: dup failed");
        return;
    }
    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("smash error: dup2 failed");
        close(shellFd);
        return;
    }
    close(fd);
    const char *cmd_line = lhs.c_str();
    SmallShell::getInstance().executeCommand(cmd_line);

    if (dup2(shellFd, STDOUT_FILENO) == -1) {
        perror("smash error: dup2 failed");
    }
    close(shellFd);
}

void PipeCommand::execute() {
    string cmd_s = _trim(string(this->cmd_line));
    string op;
    size_t pos = cmd_s.find(">>");
    if (pos != string::npos) {
        op = "|&";
    } else {
        pos = cmd_s.find("|");
        if (pos != string::npos) {
            op = "|";
        }
    }
    string lhs = _trim(cmd_s.substr(0, pos));
    string rhs = _trim(cmd_s.substr(pos + op.length()));
    if (lhs.empty() || rhs.empty()) {
        std::cerr << "smash error: pipe: invalid arguments\n";
        return;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("smash error: pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("smash error: fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    if (pid1 == 0) {
        setpgrp();
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(errno);
        }
        if (op == "|&" && dup2(fd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(errno);
        }
        close(fd[0]);
        close(fd[1]);

        Command *cmd = SmallShell::getInstance().CreateCommand(lhs.c_str());
        cmd->execute();
        exit(errno);
    }

    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("smash error: fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, nullptr, 0); // Wait for first child to avoid zombie
        return;
    }
    if (pid2 == 0) {
        setpgrp();
        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(errno);
        }
        close(fd[0]);
        close(fd[1]);

        Command *cmd = SmallShell::getInstance().CreateCommand(rhs.c_str());
        cmd->execute();
        exit(errno);
    }
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}

struct linux_dirent64 {
    ino64_t d_ino;
    off64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

long calculate_disk_usage(const std::string &path) {

    int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        return -1;
    }

    const int BUF_SIZE = 4096; // Increased buffer size for better performance
    char buf[BUF_SIZE];
    long total_kb = 0;

    struct stat dir_stat;
    if (lstat(path.c_str(), &dir_stat) == 0) {
        total_kb += (dir_stat.st_blocks * 512) / 1024;
    }
    while (true) {
        int nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1) {
            close(fd);
            return -1;
        }

        if (nread == 0) {
            break; // End of directory
        }

        for (int bpos = 0; bpos < nread;) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)(buf + bpos);
            std::string name = d->d_name;

            // Skip . and .. directories - CRITICAL FIX: Was incorrectly skipping everything except . and ..
            if (name == "." || name == "..") {
                bpos += d->d_reclen;
                continue;
            }

            std::string full_path = path;
            if (path[path.length() - 1] != '/') {
                full_path += "/";
            }
            full_path += name;

            struct stat st;
            if (lstat(full_path.c_str(), &st) == 0) {
                total_kb += (st.st_blocks * 512) / 1024;

                if (S_ISDIR(st.st_mode)) {
                    long subdir_size = calculate_disk_usage(full_path);
                    if (subdir_size >= 0) {
                        total_kb += subdir_size;
                    }
                }
            }

            bpos += d->d_reclen;
        }
    }

    close(fd);
    return total_kb;
}

void DiskUsageCommand::execute() {

    char *argv[COMMAND_MAX_ARGS];
    int argc = _parseCommandLine(this->cmd_line, argv); // Fixed: cmd*line -> cmd_line

    if (argc > 2) {
        std::cerr << "smash error: du: too many arguments" << std::endl;
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        return;
    }
    std::string path = (argc == 1) ? "." : argv[1];

    long size_kb = calculate_disk_usage(path);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }

    if (size_kb < 0) {
        std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
        return;
    }
    std::cout << "Total disk usage: " << size_kb << " KB" << std::endl;
}


void WhoAmICommand::execute() {
    const uid_t my_uid = geteuid();
    const std::string uid_text = std::to_string(my_uid);
    std::string username;
    std::string home;

    if (const char *env_home = std::getenv("HOME")) {
        home = env_home;
    }
    int fd = open("/etc/passwd", O_RDONLY);
    if (fd >= 0) {
        const int BUF_SIZE = 4096;
        char buffer[BUF_SIZE];
        ssize_t bytes_read;
        std::string partial_line;

        while ((bytes_read = read(fd, buffer, BUF_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            std::string current_chunk(buffer);

            size_t pos = 0;
            size_t newline_pos;

            if (!partial_line.empty()) {
                newline_pos = current_chunk.find('\n');
                if (newline_pos != std::string::npos) {
                    partial_line += current_chunk.substr(0, newline_pos);

                    if (!partial_line.empty() && partial_line[0] != '#') {
                        std::istringstream iss(partial_line);
                        std::string field, name, uid_field, homedir;

                        std::getline(iss, name, ':');
                        std::getline(iss, field, ':');
                        std::getline(iss, uid_field, ':');
                        std::getline(iss, field, ':');
                        std::getline(iss, field, ':');
                        std::getline(iss, homedir, ':');

                        if (uid_field == uid_text) {
                            username = name;
                            if (home.empty()) {
                                home = homedir;
                            }
                            break;
                        }
                    }
                    pos = newline_pos + 1;
                    partial_line.clear();
                } else {
                    partial_line += current_chunk;
                    continue;
                }
            }
            while ((newline_pos = current_chunk.find('\n', pos)) !=
                   std::string::npos) {
                std::string line = current_chunk.substr(pos, newline_pos - pos);

                if (!line.empty() && line[0] != '#') {
                    std::istringstream iss(line);
                    std::string field, name, uid_field, homedir;

                    std::getline(iss, name, ':');
                    std::getline(iss, field, ':');
                    std::getline(iss, uid_field, ':');
                    std::getline(iss, field, ':');
                    std::getline(iss, field, ':');
                    std::getline(iss, homedir, ':');

                    if (uid_field == uid_text) {
                        username = name;
                        if (home.empty()) {
                            home = homedir;
                        }
                        close(fd);
                        std::cout << username << " " << home << std::endl;
                        return;
                    }
                }

                pos = newline_pos + 1;
            }
            if (pos < current_chunk.length()) {
                partial_line = current_chunk.substr(pos);
            }
        }
        close(fd);
    }

    if (username.empty()) {
        username = uid_text;
    }
    std::cout << username << " " << home << std::endl;
}


