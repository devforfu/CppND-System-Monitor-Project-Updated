#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <numeric>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

using JiffsCpu = std::array<long, LinuxParser::CPUStates::kTotal_>;

JiffsCpu ReadSystemJiffsFromString(std::string line) {
  std::istringstream stream(line);
  std::string prefix;
  stream >> prefix;
  JiffsCpu jiffs;
  for (auto &jiff : jiffs) stream >> jiff;
  return jiffs;
}

JiffsCpu ReadSystemJiffsFromFile(std::string filename) {
  std::ifstream stream(filename);
  if (!stream.is_open()) return {};
  string allCpuLine;
  std::getline(stream, allCpuLine);
  return ReadSystemJiffsFromString(allCpuLine);
}


std::vector<std::string> ReadLines(std::string filename) {
  vector<string> lines{};
  std::ifstream stream(filename);
  if (!stream.is_open()) return {};
  std::string line;
  while (std::getline(stream, line)) lines.push_back(line);
  return lines;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float getMemEntry(std::string line) {
  string prefix;
  long number;
  std::istringstream s(line);
  s >> prefix >> number;
  return number;
}

float LinuxParser::MemoryUtilization() { 
  auto lines = ReadLines(kProcDirectory + kMeminfoFilename);
  auto memTotalLine = lines[0];
  auto memFreeLine = lines[1];
  float memTotal{getMemEntry(memTotalLine)}; 
  float memFree{getMemEntry(memFreeLine)};
  return (memTotal - memFree)/memTotal;
}

long LinuxParser::UpTime() { 
  auto line = ReadLines(kProcDirectory + kUptimeFilename).front();
  std::istringstream s(line);
  float uptime{0};
  s >> uptime;
  return static_cast<int>(uptime);
}

long LinuxParser::Jiffies() { 
  auto jiffs = ReadSystemJiffsFromFile(kProcDirectory + kStatFilename);
  return std::accumulate(jiffs.begin(), jiffs.end(), 0);
}

long LinuxParser::ActiveJiffies(int pid) { 
  int fieldsToSkip{13};
  std::string sink;

  auto line = ReadLines(kProcDirectory + std::to_string(pid) + kStatFilename).front();
  std::istringstream stream(line);
  for (int i = 0; i < fieldsToSkip; ++i) stream >> sink;
  
  long utime{0}, stime{0}, cutime{0}, cstime{0};
  stream >> utime >> stime >> cutime >> cstime;
  return utime + stime + cutime + cstime;
}

long LinuxParser::ActiveJiffies() { 
  using LinuxParser::CPUStates;
  auto jiffs = ReadSystemJiffsFromFile(kProcDirectory + kStatFilename);
  std::array<int, 6> cpuStates {kUser_, kNice_, kSystem_, kIRQ_, kSoftIRQ_, kSteal_};
  long totalJiffs{0};
  for (const auto state : cpuStates) totalJiffs += jiffs[state];
  return totalJiffs;
}

long LinuxParser::IdleJiffies() { 
  auto jiffs = ReadSystemJiffsFromFile(kProcDirectory + kStatFilename);
  return jiffs[CPUStates::kIdle_];
}

float LinuxParser::CpuUtilization() { 
  return LinuxParser::ActiveJiffies() / LinuxParser::Jiffies();
}

template<typename T> 
T ParseProcStatValue(std::string lookedUpKey, std::string procFile) {

  std::ifstream fstream(LinuxParser::kProcDirectory + procFile);
  
  if (fstream.is_open()) {
    std::string line;
  
    while (std::getline(fstream, line)) {
      std::istringstream stream(line);
      std::string key;
      stream >> key;
      if (key == lookedUpKey) {
        T value;
        stream >> value;
        return value;
      }
    }
  }

  return {};
}

int LinuxParser::TotalProcesses() { 
  return ParseProcStatValue<int>("processes", kStatFilename);
}

int LinuxParser::RunningProcesses() {
   return ParseProcStatValue<int>("procs_running", kStatFilename);
}

string LinuxParser::Command(int pid) { 
  return ReadLines(kProcDirectory + std::to_string(pid) + kCmdlineFilename).front();
}

string LinuxParser::Ram(int pid) { 
  auto ramKb = ParseProcStatValue<int>("VmSize:", std::to_string(pid) + kStatusFilename);
  auto ramMb = static_cast<int>(ramKb / 1024);
  return std::to_string(ramMb);
}

string LinuxParser::Uid(int pid) {
  return ParseProcStatValue<std::string>("Uid:", std::to_string(pid) + kStatusFilename);
}

string LinuxParser::User(int pid) {
  auto uid = LinuxParser::Uid(pid);

  auto getUsernameIfMatch = [uid](std::string line) -> std::optional<std::string> {
    if (line.find(uid) == std::string::npos) return {};
    auto sep = line.find(":");
    return {line.substr(0, sep)};
  };

  for (const auto &line : ReadLines(kPasswordPath)) {
    if (auto maybeMatch = getUsernameIfMatch(line)) {
      auto& username = *maybeMatch;
      return username;
    }
  }

  return string();
}

long LinuxParser::UpTime(int pid) {
  auto lines = ReadLines(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (lines.empty()) return 0;

  std::istringstream stream(lines.front());
  std::vector<std::string> values(
    std::istream_iterator<std::string>{stream},
    std::istream_iterator<std::string>{}
  );
  
  if (values.size() < 22) return 0;

  long startTime = std::stol(values[21]);
  long systemUpTime = LinuxParser::UpTime();
  long hz = sysconf(_SC_CLK_TCK);
  
  return systemUpTime - startTime/hz;
}
