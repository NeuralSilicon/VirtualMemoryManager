#ifndef FIFO_hpp
#define FIFO_hpp

#include <stdio.h>
#include "DiskHandler.hpp"
#include <sys/wait.h>

class FIFO {
private:
    Entry entry;
    DiskHandler disk;
    void init();
    void deactivateProcess(int pid, map<int,queue<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,queue<pair<int, int>>> *frame);
public:
    FIFO(const Entry &entry);
};

#endif /* FIFO_hpp */
