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
      int x;
  }



  if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0){
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0 ) {
 //   return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0){
      return new ChangePromptCommand(cmd_line);
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

  else {
  //  return new ExternalCommand(cmd_line);
  }

    return nullptr;

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
}

void SmallShell::printAliases() const {
    for(const auto &pair: this->aliases){
        cout << pair.first <<"='" <<pair.second <<"'" << endl;
    }
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

void GetCurrDirCommand::execute() {
    int size = 100;
    char *pathName = new char(size);
    ///Allocate more memory if the path is too long
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
        cout << "[" << pair.first <<"] " << pair.second.cmd_line;
    }
}

void JobsList::killAllJobs() {
    for (const auto& pair : job_map){
        kill(pair.second.pid, SIGKILL);
    }
    job_map.clear();
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
            cout << pair.second.pid <<": " << pair.second.cmd_line << endl;
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
