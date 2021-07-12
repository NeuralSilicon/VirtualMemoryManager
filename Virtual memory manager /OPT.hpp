#ifndef OPT_hpp
#define OPT_hpp

#include <stdio.h>
#include <list>
#include "DiskHandler.hpp"
#include <sys/wait.h>

class OPT {
private:
    pair<int,int> findOPT(int index, map<int,list<pair<int, int>>> frame);
    Entry entry;
    DiskHandler disk;
    void init();
    void deactivateProcess(int pid, map<int,list<pair<int, int>>> *frame);
    void activateProcess(int pid, map<int,list<pair<int, int>>> *frame);
public:
    OPT(const Entry &entry);
};
#endif /* OPT_hpp */
