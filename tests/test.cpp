#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include "format.h"
#include "linux_parser.h"

void testFormat() {
    struct TestCase {
        long seconds;
        std::string time;
    };
    std::vector<TestCase> tests = {
        {.seconds = 61, .time = std::string("00:01:01")},
        {.seconds = 3662, .time = std::string("01:01:02")},
        {.seconds = 1439 * 60 + 59, .time = std::string("23:59:59")},
        {.seconds = 0, .time = std::string("00:00:00")},
    };
    for (const auto &testCase : tests) {
        auto elapsed = Format::ElapsedTime(testCase.seconds);
        std::cout << "expected=" << testCase.time << ", actual=" << elapsed << "\n"; 
        assert(elapsed == testCase.time);
    }
}

void testLinuxParser() {
    float value = LinuxParser::MemoryUtilization();
    std::cout << "mem util: " << std::setprecision(4) << value << "\n";
    assert(value > 0);

    long jiffies = LinuxParser::Jiffies();
    std::cout << "total jiffies: " << jiffies << "\n";
    assert(jiffies > 0);

    long pidJiffies = LinuxParser::ActiveJiffies(1);
    std::cout << "total jiffies for pid=1: " << pidJiffies << "\n";
    assert(pidJiffies > 0);

    int totalProcesses = LinuxParser::TotalProcesses();
    std::cout << "total processes: " << totalProcesses << "\n";
    assert(totalProcesses > 0);

    int runningProcesses = LinuxParser::RunningProcesses();
    std::cout << "running processes: " << runningProcesses << "\n";
    assert(runningProcesses > 0);

    auto cmd = LinuxParser::Command(1);
    std::cout << "pid=1 command: " << cmd << "\n";
    assert(!cmd.empty());

    auto ram = LinuxParser::Ram(1);
    std::cout << "pid=1 ram: " << ram << "\n";
    assert(!ram.empty());

    auto username = LinuxParser::User(1);
    std::cout << "pid=1 username: " << username << "\n";
    assert(username == "root");

    auto pidUpTime = LinuxParser::UpTime(1);
    std::cout << "pid=1 uptime: " << pidUpTime << "\n";
    assert(pidUpTime > 0);
}

int main() {
    testFormat();
    testLinuxParser();
    return 0;
}
