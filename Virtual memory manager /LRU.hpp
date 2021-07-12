#ifndef LRU_hpp
#define LRU_hpp

#include <stdio.h>
#include <list>
#include "DiskHandler.hpp"
#include <sys/wait.h>

class LRU {
private:
    pair<int,int> findLRU(int index, map<int,list<pair<int, int>>> frame);
    Entry entry;
    DiskHandler disk;
    void init();
    void deactivateProcess(int pid, map<int,list<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,list<pair<int, int>>> *frame);
public:
    LRU(const Entry &entry);
};

#endif /* LRU_hpp */
