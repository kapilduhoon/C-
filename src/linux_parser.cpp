#include <dirent.h>
#include <unistd.h>
#include <cmath>
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <vector>

#include "format.h"
#include "linux_parser.h"

using std::stof;
using std::stoi;
using std::stol;
using std::string;
using std::to_string;
using std::vector;
using std::istringstream;
using std::istream_iterator;
using std::ifstream;

// Helper functions
vector<string> string_to_vec(const string& line) {
  istringstream iss(line);
  istream_iterator<string> beg(iss), end;
  vector<string> values(beg, end);
  return values;
}

string LinuxParser::extract_match(const string& directory,
                                  const string& to_match) {
  ifstream filestream(directory);
  string val;
  string line;
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      vector<string> values = string_to_vec(line);
      if (!to_match.compare(0, to_match.size(), values[0])) {
        val = values[1];
        break;
      }
    }
  }
  return val;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
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
  string os, kernel;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  const string total = "MemTotal";
  const string free = "MemFree";
  string terms[] = {total, free};

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  string line;
  string term;
  float mem;
  std::map<string, float> mp;

  if (filestream.is_open()) {
    for (auto term : terms) {
      while (getline(filestream, line)) {
        vector<string> values = string_to_vec(line);
        if (term.compare(0, values[0].size(), values[0])) {
          mem = stof(values[1]);
          mp[term] = mem;
          break;
        }
      }
    }
  }
  return (mp[total] - mp[free]) / mp[total];
}

// TODO: Read and return the system uptime
long int LinuxParser::UpTime() {
  string line;
  string uptime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return stol(uptime);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  float utime, stime, cutime, cstime, starttime;
  const string directory = kProcDirectory + to_string(pid) + kStatFilename;

  std::ifstream filestream(directory);
  string line;
  getline(filestream, line);
  vector<string> values = string_to_vec(line);

  utime = std::stof(values[kUser_]);
  stime = std::stof(values[14]);
  cutime = std::stof(values[15]);
  cstime = std::stof(values[16]);
  starttime = std::stol(values[21]);

  long int uptime = LinuxParser::UpTime();
  float hertz = sysconf(_SC_CLK_TCK);

  float total_time = utime + stime + cutime + cstime;
  float seconds = uptime - (starttime / hertz);

  return ((total_time / hertz) / seconds) * 100;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> all_jiffies = LinuxParser::CpuUtilization();
  float sum = 0;
  all_jiffies.erase(all_jiffies.begin() + 3);
  for (auto j : all_jiffies) sum += std::stof(j);
  return sum;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> all_jiffies = LinuxParser::CpuUtilization();
  return std::stof(all_jiffies[kIdle_] + all_jiffies[kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  const string directory = kProcDirectory + kStatFilename;
  std::ifstream filestream(directory);
  string line;
  vector<string> numbers;
  getline(filestream, line);
  vector<string> values = string_to_vec(line);
  for (size_t i = 1; i < values.size(); i++) numbers.push_back(values[i]);
  return numbers;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  const string directory = kProcDirectory + kStatFilename;
  const string proc = "processes";
  string val = LinuxParser::extract_match(directory, proc);
  return stoi(val);
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  const string directory = kProcDirectory + kStatFilename;
  const string rproc = "procs_running";
  string val = LinuxParser::extract_match(directory, rproc);
  return stoi(val);
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string cmd;
  const string directory =
      kProcDirectory + std::to_string(pid) + kCmdlineFilename;
  std::ifstream filestream(directory);
  getline(filestream, cmd);
  vector<string> values = string_to_vec(cmd);
  for (auto v : values) cmd.append(v);
  return cmd;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  int ram;
  const string directory =
      kProcDirectory + std::to_string(pid) + kStatusFilename;
  const string vms = "VmSize:";
  ram = std::stoi(LinuxParser::extract_match(directory, vms)) / 1024;
  return std::to_string(ram);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string uid;
  const string directory =
      kProcDirectory + std::to_string(pid) + kStatusFilename;
  const string match = "Uid:";
  uid = LinuxParser::extract_match(directory, match);
  return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  const string directory = kPasswordPath;
  string uid = "x:" + LinuxParser::Uid(pid);
  std::ifstream filestream(directory);
  string line;
  string user;
  std::size_t found;
  while (getline(filestream, line)) {
    found = line.find(uid);
    if (found != std::string::npos) {
      found = line.find(":");
      return line.substr(0, found);
    }
  }
  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  const string directory = kProcDirectory + to_string(pid) + kStatFilename;
  std::ifstream filestream(directory);
  string line;
  getline(filestream, line);
  vector<string> values = string_to_vec(line);
  long ticks = std::stol(values[21]);
  return LinuxParser::UpTime() - (ticks / sysconf(_SC_CLK_TCK));
}