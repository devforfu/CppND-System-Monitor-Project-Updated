#include "process.h"
#include "linux_parser.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid):
  pid_{pid}, 
  user_(LinuxParser::User(pid)),
  cmd_(LinuxParser::Command(pid)) 
{  
}

int Process::Pid() const { return pid_; }

string Process::Command() const { return cmd_; }

string Process::User() const { return user_; }

float Process::CpuUtilization() { return 0; }

string Process::Ram(){ return LinuxParser::Ram(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

bool Process::operator<(Process const& a) const { return pid_ < a.Pid(); }