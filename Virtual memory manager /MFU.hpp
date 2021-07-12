#ifndef MFU_hpp
#define MFU_hpp

#include <stdio.h>
#include <list>
#include "DiskHandler.hpp"
#include <sys/wait.h>

class MFU {
private:
    pair<int,int> findMFU(int index, map<int,list<pair<int, int>>> frame);
    Entry entry;
    DiskHandler disk;
    void init();
    void deactivateProcess(int pid, map<int,list<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,list<pair<int, int>>> *frame);
public:
    MFU(const Entry &entry);
};
#endif /* MFU_hpp */
