#ifndef PROCESSPASER_H
#define PROCESSPASER_H

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"


using namespace std;

class ProcessParser{
private:
    ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static string getVmSize(string pid);
    static string getCpuPercent(string pid);
    static long int getSysUpTime();
    static string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static size_t getNumberOfCores();
    static string PrintCpuStats(vector<string> values1, vector<string>values2);
    static bool isPidExisting(string pid);
    static float getSysActiveCpuTime(vector<string> values);
    static float getSysIdleCpuTime(vector<string> values);

};

// TODO: Define all of the above functions below:

/**
 * @function: det the command line of the process.
 * @param: string pid
 * @return string
 */
string ProcessParser::getCmd(string pid) {
    string line;
    std::ifstream  instream;
    Util::getStream((Path::basePath() + pid + Path::cmdPath()), instream);
    std::getline(instream, line);
    return line;
}

/**
 * @function: get all the pid on the system right now.
 * @param: void.
 * @return: vector<string>
 */
vector<string> ProcessParser::getPidList() {
    DIR *dir;
    vector<string> pid_list;
    //error handling for open()
    if(!(dir=opendir("/proc")))    
        throw std::runtime_error(std::strerror(errno));
    // walking through the /proc file, and filtering for the pid directories.
    while (dirent *dirptr = readdir(dir)) {
        if(dirptr->d_type != DT_DIR) 
            continue;
        if(all_of(dirptr->d_name, dirptr->d_name+std::strlen(dirptr->d_name), 
                [](char c){return std::isdigit(c); })) {
            pid_list.push_back(dirptr->d_name); 
        }
    }
    if (closedir(dir))
        throw std::runtime_error(std::strerror(errno));

    return pid_list;
}

/**
 * @function: get the virtual memory size of the process pid.
 * @param: string pid.
 * @return string.
 */
string ProcessParser::getVmSize(string pid) {
    string line;
    string name = "VmData";
    float result = 0.0f;
    ifstream instream;
    Util::getStream((Path::basePath() + pid + Path::statusPath()), instream);
    while(std::getline(instream, line)) {
    // searching for the VmData  
        if(line.compare(0, name.size(), name) == 0) {
        // sliceing the string into words verctor 
        istringstream buf(line);
        istream_iterator<string> beg(buf), end;
        vector<string>  values(beg, end);
        result = stof(values[1]) / float(1024);
        break;
        }
    }
    return to_string(result);
}

/**
 * @function: get the uptime of the process pid.
 * @param: string pid.
 * @return: string 
 */
string ProcessParser::getProcUpTime(string pid) {
    string line;
    string result;
    ifstream instream;
    Util::getStream((Path::basePath() + pid + '/' + Path::statPath()), instream);
    getline(instream, line);
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    
    result = to_string(float(stof(values[13]) / sysconf(_SC_CLK_TCK)));
    return result;
}

/**
 * @function: get the system cpu time
 * @param: void
 * @return: long int
 *
 */
long int ProcessParser::getSysUpTime() {
    string line;
    ifstream instream;
    long int result;
    Util::getStream((Path::basePath() + Path::upTimePath()), instream);
    getline(instream, line);
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    result =  stoi(values[0]); 
    return result;

}

/** 
 * @function: get the cpu percentage of the process pid. 
 * @param: string pid.
 * @return string , the percentages of the whole cpu time.
 */
string ProcessParser::getCpuPercent(string pid) {
    string line;
    string value;
    float result;
    ifstream instream;
    Util::getStream((Path::basePath() + pid + "/" + Path::statPath()), instream);
    getline(instream, line); 
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();

    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float system_seconds = uptime - starttime/freq;
    result = 100.0f * ((total_time/freq) / system_seconds);
    return to_string(result);

}

/**
 * @function: get the user name of the process pid.
 * @param: string pid.
 * @return: string.
 */
string ProcessParser::getProcUser(string pid) {
   string line;
   string result = " ";
   string name = "Uid:";
   ifstream instream;
   Util::getStream((Path::basePath() + pid + Path::statusPath()), instream);
   while(getline(instream, line)) {
       if(line.compare(0, name.size(), name) == 0) {
           istringstream buf(line);
           istream_iterator<string> beg(buf), end;
           vector<string> values(beg, end);

           result = values[1];
           break;
       }
   }
   // to retrieve user name by UID form /etc/passwd
   Util::getStream("/etc/passwd", instream);
   name = ("x:" + result);
   while(getline(instream, line)) {
        if(line.find(name) != string::npos) { 
            result = line.substr(0, line.find(":"));
            return result;
        }
   }
   return " ";
}

/**
 * @function: get the system cpu information list  of core i or overall.
 * @param: core i
 * @return : vector<string> ,percentage
 */
vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    string line;
    string name = "cpu" + coreNumber;
    ifstream instream;
    Util::getStream((Path::basePath() + Path::statPath()), instream);
    while(getline(instream, line)) {
        if(name.compare(0, name.size(), name) == 0) {
            istringstream buf(line); 
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values;
        } 
    }
    return (vector<string>() );
} 

/**
 * @function: get the system ram using percentage.
 * @param: void
 * @return: flaot
 */
float ProcessParser::getSysRamPercent() {
    string line;
    string name1 = "MemFree:";
    string name2 = "MemAvailable:";
    string name3 = "Buffers:";
    ifstream instream;

    float free_mem = 0;
    float total_mem = 0;
    float buffers = 0;
    float result;
    
    Util::getStream((Path::basePath() + Path::memInfoPath()), instream); 
    while(getline(instream, line)) {
        if(free_mem !=0 && total_mem != 0) 
            break;
        
        if(line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        
        if(line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
    }
        
        if(line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    
    result = float(100.0 * (1 - free_mem / (total_mem-buffers)));
    return result;
}

/**
 * @function: get the system kernel version.
 * @param: void
 * @return: string 
 */
string ProcessParser::getSysKernelVersion() {
    string line;
    ifstream instream;
    string name = "Linux version";
    string result = "";
    
    Util::getStream((Path::basePath() + Path::versionPath()), instream);    
    while(getline(instream, line)) {
        if(line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = values[2];
        }
    }
    return result;
}

/**
 * @function: get the total threds all arocss all process.
 * @param: void.
 * @return int.
 */
int ProcessParser::getTotalThreads() {
    string line;
    string name = "Threds";
    int result = 0;
    ifstream insteam;
    vector<string> pid_list = ProcessParser::getPidList();

    for(auto pid : pid_list) {
        Util::getStream((Path::basePath() + pid + Path::statusPath()), insteam);
        while(getline(insteam, line)) {
            if(line.compare(0, name.size(), name) == 0) {
                istringstream buf(line);
                istream_iterator<string> beg(buf), end;
                vector<string> values(beg, end);
                result += stoi(values[1]);
                break;
            }
        }
    }
    return result;
}

/**
 * @function: get the total numbers of process.
 * @param: void
 * @return: int.
 */
int ProcessParser::getTotalNumberOfProcesses() {
    string line;
    ifstream instream;
    string name = "processes";
    int result = 0;
    Util::getStream((Path::basePath() + Path::statPath()), instream);
    
    while(getline(instream, line)) {
        if(line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

/**
 * @function: get the numbers of the running processes
 * @param: void
 * @return: int
 */
int ProcessParser::getNumberOfRunningProcesses() {
    string line;
    ifstream instream;
    string name = "procs_running";
    int result = 0;
    Util::getStream((Path::basePath() + Path::statPath()), instream);
    
    while(getline(instream, line)) {
        if(line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

/**
 * @function: get the name of the os
 * @param: void
 * @return: string
 */
string ProcessParser::getOSName() {
    string line;
    string name = "PRETTY_NAME";
    ifstream instream;
    string result = "";
    
    Util::getStream("/etc/os-release", instream);    
    while(getline(instream, line)) {
        if(line.compare(0, name.size(), name) == 0) {
            // piont to the os name
            size_t index = line.find('=');
            ++index;
            result = line.substr(index); 
            // erase the qoute between the os name.
            result.erase(remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }
    return result;
}

/**
 * @function: get the core numbers of the system.
 * @param: void
 * @return: int
 */
size_t ProcessParser::getNumberOfCores() {
    string line;
    int result = 0;
    string name = "cpu cores";
    ifstream instream;
    
    Util::getStream((Path::basePath() + "cpuinfo"), instream);
    while(getline(instream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = (stoi(values[3]));
            return result;
        }
    } 
    return result;
}

/**
 * @function: calculate the cpu usage in the span of short time.  
 * @param: vector<string> value1(prevoius), vector<string> value2(current)
 * @return: string
 */
string ProcessParser::PrintCpuStats(vector<string> values1, vector<string>values2) {
    float active_time = ProcessParser::getSysActiveCpuTime(values2) - ProcessParser::getSysActiveCpuTime(values1);
    float idle_time = ProcessParser::getSysIdleCpuTime(values2) - ProcessParser::getSysIdleCpuTime(values1);
    float result = 100.0f * (active_time / (active_time+idle_time));
    return to_string(result);
}

/**
 * @function: checking wether the pid exsit.
 * @param: string pid
 * @return: bool
 */
bool ProcessParser::isPidExisting(string pid) {
    bool result = false;

    vector<string> pid_list = ProcessParser::getPidList();
    for(auto line : pid_list) {
        if(line.compare(0, pid.size(), pid) == 0) {
            result = true; 
            break;
        } 
    }
    return result;
}

/**
 * @function: get the cpu active time from cpu information vector .
 * @param:  vector<string> value. the cpu information string vector.
 * @return: float. cpu active time.
 */
float ProcessParser::getSysActiveCpuTime(vector<string> values) {
    return (stof(values[S_USER]) + 
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]) );
}

/**@function: get the cpu idle time.
 * @param: vector<string> values. teh cpu information string.
 * @return: flaot.
 */
float ProcessParser::getSysIdleCpuTime(vector<string> values) {
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));

}

#endif
