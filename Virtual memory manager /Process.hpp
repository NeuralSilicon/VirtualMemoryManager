#ifndef Process_hpp
#define Process_hpp

#include <stdio.h>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <list>
#include <map>
#include <cstring>
using namespace std;

struct Process {
    //active or deactive
    bool active = true;
    //id for this process
    int pid = 0;
    //size of process
    int size = 0;
    //number of page fault
    int pageFault = 0;
    //number of page replacement
    int pageReplacement = 0;

    
    Process(){
        active = true; pid = 0;size = 0;pageFault = 0; pageReplacement = 0;
    }
    
    //check process ID
    bool operator = (const Process &RHS){
        return (this->pid == RHS.pid);
    }
    
    //print this process info
    void print(){
        std::cout<<"Process "<<pid<<" : "<<std::endl;
        std::cout<<"Number of page faults: "<<pageFault<<std::endl;
        std::cout<<"Number of page Replacement: "<<pageReplacement<<std::endl;
    }
};

struct MemoryAddress {
private:
    //process id for this address
    int pid = 0;
    //store the binary value
    string binaryValue = "";
    //segment number
    int segmentNumber = 0;
    //page number
    int pageNumber = 0;
    //page offset
    int pageOffset = 0;

    
    //find the power number
    int powerFor(int value, int power = -1){
        if(value == 0){
            return power;
        }
        return powerFor(value/2, power + 1);
    }
    
    //convert binary to decimal
    int binaryToDecimal(string str)
    {
        return std::stoi(str, nullptr, 2);
    }
public:
    void print(){
        cout<<segmentNumber<<pageNumber<<endl;
    }
    
    //get process
    int getPid(){
        return this->pid;
    }
    
    //binary value
    string getBinary(){return this->binaryValue;}
    void setBinary(string value, int pid){
        this->binaryValue = value;
        this->pid = pid;
    }
    
    //get address values
    pair<int, int> getAddressValues(){ return {segmentNumber, pageNumber};}
    
    //extract the info from our binary based on the given sizes
    void extractValues(int maxSegment, int pageSize){
        
        //if our binary value is -1 then dont continue
        if(binaryValue == "-1"){
            return;
        }
        
        // 16 maximum segment length (in number of pages) - > 2^4 = 4 bit page number
        int pageNumberBit = this->powerFor(maxSegment);
        //256 //The page size is typically a power of 2, 2^𝑛, in this case 2^𝑛=256⇒𝑛=8. 8 bit is the offset or displacement
        int pageOffsetBit = this->powerFor(pageSize);
        //in this case address is 16bit then 16 - 8 - 4  = 4 bit for segment
        int segmentBit = int(binaryValue.size()) - pageNumberBit - pageOffsetBit;
        
        //substr binary size - page offset bit - 1
        this->pageOffset = this->binaryToDecimal(this->binaryValue.substr(binaryValue.size() - pageOffsetBit, pageOffsetBit));
        
        //substr binary size - page offset bit - page number bit - 1
        this->pageNumber = this->binaryToDecimal(this->binaryValue.substr(binaryValue.size() - pageOffsetBit - pageNumberBit, pageNumberBit));
        
        //substr 0 to segment bit
        this->segmentNumber = this->binaryToDecimal(this->binaryValue.substr(0, segmentBit));
    }
    
};

struct Entry {
    // total No. of page frames in main memory
    int memoryPageFrames = 0;
    // maximum segment length (in number of pages)
    int maxSegmentSize = 0;
    // page size (in number of bytes)
    int pageSize = 0;
    // No. of page frames per process for FIFO, LIFO, LRU-X, LDF...
    int numberOfPageFrames = 0;
    // used by LRU-X, OPT-X for lookaheads
    int lookAhead = 0;
    
    // min pool size
    int minPool = 0;
    // max pool size
    int maxPool = 0;
    //current pool
    int currentPool = 0;
    
    // total number of processes
    int numberOfProcess = 0;
    //working set, pid with active mem address
    map<int,list<pair<int, int>>> workingSet;
    //processes map<pid, process>
    map<int, Process> process = map<int, Process>();
    //all the memory address
    vector<MemoryAddress> memoryAddress;
    
    //insert values based on indice when arg is being parsed
    void insertValues(int index, string value){
        switch (index) {
            case 0:
                memoryPageFrames = stoi(value);
                break;
            case 1:
                maxSegmentSize = stoi(value);
                break;
            case 2:
                pageSize =  stoi(value);
                break;
            case 3:
                numberOfPageFrames =  stoi(value);
                break;
            case 4:
                lookAhead =  stoi(value);
                break;
            case 5:
                minPool =  stoi(value);
                break;
            case 6:
                maxPool =  stoi(value);
                break;
            case 7:
                numberOfProcess =  stoi(value);
                break;
            default:
                break;
        }
    }
};

#endif /* Process_hpp */
