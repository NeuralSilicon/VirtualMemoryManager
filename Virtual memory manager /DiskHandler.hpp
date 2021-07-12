#ifndef DiskHandler_hpp
#define DiskHandler_hpp

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <queue>
#include <map>
#include "Process.hpp"

class DiskHandler {
    //hold the values in a hashtable for faster access
    map<string, MemoryAddress> memoryAddress;
public:
    DiskHandler(){memoryAddress = map<string, MemoryAddress>();}
    DiskHandler(const vector<MemoryAddress> &memoryAddress);
    
    //make a request;
    void makeRequest(string binary, int sid, int (*proc)[2]);
};

#endif /* DiskHandler_hpp */
