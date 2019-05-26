#ifndef PROCESSCONTAINER_H
#define PROCESSCONTAINER_H

#include "Process.h"
#include <vector>
class ProcessContainer{

private:
    std::vector<Process>_list;
public:
    ProcessContainer(){
        this->refreshList();
    }
    void refreshList();
    std::string printList();
    std::vector<std::vector<std::string>> getList();
};

/**
 * @function: refresh the process class list on host.
 * @param: void
 * @return: void
 */
void ProcessContainer::refreshList(){
    std::vector<std::string> pidList = ProcessParser::getPidList();
    this->_list.clear();
    for (auto pid : pidList){
        Process proc(pid);
        if (stoi(proc.getUpTime()) > 0)
            this->_list.push_back(proc);
    }
}

/**
 * @function: get the process infomation string list whuich used to print.
 * @param: void
 * @return: string
 */
std::string ProcessContainer::printList(){
    std::string result="";
    for (auto index : this->_list){
        result += index.getProcess();
    }
    return result;
}

/**
 * @function: get all of the pocesses information string in _list and string them
 * in 2-D vector, one process a vector.
 * @param: void
 * @return: vector<vector<sring>>
 */
std::vector<std::vector<std::string> > ProcessContainer::getList(){
    std::vector<std::vector<std::string>> values;
    std::vector<std::string> stringifiedList;
    for (auto index : this->_list){
        stringifiedList.push_back(index.getProcess());
    }
    // attention! declaring the lastindex to be type size_t
    size_t lastIndex = 0;
    for (size_t i=0; i<stringifiedList.size();++i){
        if(i %10 == 0 && i > 0){
          std::vector<std::string>  sub(&stringifiedList[i-10], &stringifiedList[i]);
          values.push_back(sub);
          lastIndex = i;
        }
        // the last index of the last one set can no be fullly devided bt 10.
        if(i == (this->_list.size() - 1) && (i-lastIndex)<10){
            std::vector<std::string> sub(&stringifiedList[lastIndex],&stringifiedList[i+1]);
            values.push_back(sub);
        }
   }
    return values;
}

#endif
