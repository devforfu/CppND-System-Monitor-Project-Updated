#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
private:
    int pid_;
    std::string user_;
    std::string cmd_;

public:
    explicit Process(int pid);

    int Pid() const;
    std::string Command() const;
    std::string User() const;
    
    float CpuUtilization();
    std::string Ram();              
    long int UpTime();              
    bool operator<(Process const& a) const;
};

#endif