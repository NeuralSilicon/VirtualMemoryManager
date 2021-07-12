#ifndef LRUX_hpp
#define LRUX_hpp

#include <stdio.h>
#include <list>
#include "DiskHandler.hpp"
#include <sys/wait.h>
#include <algorithm>

class LRUX {
private:
    pair<int,int> findLRUX(int index, map<int,list<pair<int, int>>> frame);
    Entry entry;
    DiskHandler disk;
    void init();
    
    void deactivateProcess(int pid, map<int,list<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,list<pair<int, int>>> *frame);
public:
    LRUX(const Entry &entry);
};

#endif /* LRUX_hpp */
