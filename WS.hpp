#ifndef WS_hpp
#define WS_hpp

#include <stdio.h>
#include <list>
#include "DiskHandler.hpp"
#include <sys/wait.h>

class WS {
private:
    Entry entry;
    DiskHandler disk;
    void init();
    void incrementWS(pair<int, int> address, int pid);
    void removeWS(int pid);
    void print();
    int concurrentMax = 0; //used to follow the workingSet
    int max = 0; //record the max size
    int min = 0; //record the min size
    
    void deactivateProcess(int pid, map<int,list<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,list<pair<int, int>>> *frame);
public:
    WS(const Entry &entry);
};

#endif /* WS_hpp */
