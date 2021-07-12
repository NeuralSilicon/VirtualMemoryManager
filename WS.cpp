#include "WS.hpp"

//Copy the process entry and init the WS
WS::WS(const Entry &entry){
    this->entry = entry;
    
    //copy the memoryaddress into disk for I/O op
    disk = DiskHandler(entry.memoryAddress);
    cout<<"WS: "<<endl;
    //init our WS algo
    init();
};


//init our WS algorithm
void WS::init(){
    
    //init semaphore for disk request
    int segmet(key_t key, int nsems, int semflag);
    static key_t key;
    int sid = semget(key, 1, 0666 | IPC_CREAT);
    
    //create semaphore for page replacement and process activation
    int b1 = semget(1525557, 1, 0666 | IPC_CREAT);
    int b2 = semget(15255572, 1, 0666 | IPC_CREAT);
    
    struct sembuf sb; sb.sem_num = 0; sb.sem_flg = 0;
    
    //frame of queues for segment#, page#
    map<int,list<pair<int, int>>> frame;
    
    //memoery index
    int memoryIndex = 0;
    
    //create pipe
    int proc[2];
    if (pipe(proc) == -1){exit(1);}; //if pipe failed
    int fid, fid2;

    //======================= iterate through the processes request ====================
    Loop: while(memoryIndex < entry.memoryAddress.size()){
        
        
        //access the process for this memory address
        int pid = entry.memoryAddress[memoryIndex].getPid();
        
        pair<int,int> address = entry.memoryAddress[memoryIndex].getAddressValues();
        
        //if memory address is -1, go to next process
        if(entry.memoryAddress[memoryIndex].getBinary() == "-1"){
            
            //increment working set
            removeWS(pid);
            
            /*
             remove from current pool
             deactive this process
             */
            entry.currentPool -= frame[pid].size();
            entry.process[pid].active = false;
            
            entry.process[pid].print();
            
            //increase memory index
            memoryIndex += 1;
            
            //go to loop
            goto Loop;
        }


        //look in our map for this mem address (segment, page)
        if(frame.find(pid) != frame.end()){
                        
            //if the size of our queue is less than the given frame
            //go ahead add the memory to it
            if(frame[pid].size() < entry.numberOfPageFrames){
                
                //check if mem add is in frame
                bool inFrame = false;
                for(auto p: frame[pid]){
                    if(p == address){
                        inFrame = true;
                    }
                }
                
                if(inFrame){
                    //first remove the duplicate from frame
                    frame[pid].remove(address);
                    
                    //add back at the end
                    frame[pid].push_back(address);
                    
                    //increase memory index
                    memoryIndex += 1;
                    
                    goto Loop;
                }
                
                //create the fork
                fid = fork();
                if (fid == -1){exit(1);}
                
                if(fid == 0){
                    //wait for b2
                    sb.sem_op = -1;
                    semop(b2, &sb, 1);
                    
                    //check for process activation and pool size
                    activateProcess(pid, &frame);
                    
                    //signal for b1
                    sb.sem_op = 1;
                    semop(b1, &sb, 1);
                    
                    exit(1);
                }else{
                    //signal b2
                    sb.sem_op = 1;
                    semop(b2, &sb, 1);
                    
                    //wait for b1
                    sb.sem_op = -1;
                    semop(b1, &sb, 1);
                
                    //add another mem address to the frame
                    frame[pid].push_back(address);
                    
                    //increment working set
                    incrementWS(entry.memoryAddress[memoryIndex].getAddressValues(), pid);
                    
                    //increase page fault
                    entry.process[pid].pageFault += 1;
                    
                    
                    int returnStatus;
                    waitpid(fid, &returnStatus, 0);
                }
            }else{
                
                //check if mem add is in frame
                bool inFrame = false;
                for(auto p: frame[pid]){
                    if(p == address){
                        inFrame = true;
                    }
                }
                
                if(inFrame){
                    //if no more space then do pagereplacement by poping the last process from frame
                    frame[pid].pop_front();
                    
                    //first remove the duplicate from frame
                    frame[pid].remove(address);
                    
                    //add back at the end
                    frame[pid].push_back(address);
                    
                    //increase memory index
                    memoryIndex += 1;
                    
                    goto Loop;
                }
                
                //create the fork
                fid = fork();
                if (fid == -1){exit(1);}
                
                if(fid == 0){
                    //wait for b2
                    sb.sem_op = -1;
                    semop(b2, &sb, 1);
                    
                    //check for process activation and pool size
                    activateProcess(pid, &frame);
                    
                    //signal for b1
                    sb.sem_op = 1;
                    semop(b1, &sb, 1);
                    
                    exit(1);
                }else{
                    //signal b2
                    sb.sem_op = 1;
                    semop(b2, &sb, 1);
                    
                    //wait for b1
                    sb.sem_op = -1;
                    semop(b1, &sb, 1);
                
                    //if no more space then do pagereplacement by poping the last process from frame
                    frame[pid].pop_front();
                    
                    fid2 = fork();
                    if(fid2 == -1){exit(1);}
                    
                    if(fid2 == 0){
                        //wait for b1
                        sb.sem_op = -1;
                        semop(b1, &sb, 1);
                       
                        //make a request to disk to bring the seg and page number
                        disk.makeRequest(entry.memoryAddress[memoryIndex].getBinary(), sid, &proc);
                       
                        exit(1);
                    }
                    else{
                        //signal for b1
                        sb.sem_op = 1;
                        semop(b1, &sb, 1);
                        
                        //wait sem until we get the disk result back
                        sb.sem_op = -1;
                        semop(sid, &sb, 1);

                        //read the pipe
                        close(proc[1]); //close write descriptor
                        read(proc[0], &address, sizeof(address)); //read
                        
                        //add another mem address to the frame
                        frame[pid].push_back(address);
                        
                        close(proc[0]); //close the read descriptor
                        
                        //increment working set
                        incrementWS(address, pid);
                        
                        //increase page fault
                        entry.process[pid].pageFault += 1;
                            
                        //increase page replacement
                        entry.process[pid].pageReplacement += 1;
                        
                        int returnStatus;
                        waitpid(fid2, &returnStatus, 0);
                        
                        //signal b2
                        sb.sem_op = 1;
                        semop(b2, &sb, 1);
                    }
                    
                    //wait b2
                    sb.sem_op = -1;
                    semop(b2, &sb, 1);
                    
                    int returnStatus;
                    waitpid(fid, &returnStatus, 0);
                }
            }
            
        }else{
            
            //create the fork
            fid = fork();
            if (fid == -1){exit(1);}
            
            if(fid == 0){
                //wait for b2
                sb.sem_op = -1;
                semop(b2, &sb, 1);
                
                //check for process activation and pool size
                activateProcess(pid, &frame);
                
                //signal for b1
                sb.sem_op = 1;
                semop(b1, &sb, 1);
                
                exit(1);
            }else{
                //signal b2
                sb.sem_op = 1;
                semop(b2, &sb, 1);
                
                //wait for b1
                sb.sem_op = -1;
                semop(b1, &sb, 1);
                
                //we didnt find the process in our map then we add the first mem address
                //to the frame for this process
                frame[pid].push_back(address);
                
                //increment working set
                incrementWS(entry.memoryAddress[memoryIndex].getAddressValues(), pid);
                
                //increase page fault
                entry.process[pid].pageFault += 1;

                
                int returnStatus;
                waitpid(fid, &returnStatus, 0);
            }
        }
        
        //adjust the current pool size
        entry.currentPool = 0;
        for(auto p: frame){
            entry.currentPool += p.second.size() ;
        }
        
        //increase memory index
        memoryIndex += 1;
    }
    
    //destory semaphore
    semctl(sid,0, IPC_RMID, 0);
    semctl(b1,0, IPC_RMID, 0);
    semctl(b2,0, IPC_RMID, 0);
    
    print();
}


//======================= Handle Algorithm Request ====================
//handle the working set of active mems in main memory
void WS::incrementWS(pair<int, int> address, int pid){
    
    //if this pid never added then add it to working set, no duplicates
    if(entry.workingSet.find(pid) == entry.workingSet.end()){
        entry.workingSet[pid].push_back(address);
    }else{
        for(auto p:entry.workingSet[pid]){
            if(p == address){
                break;
            }
        }
        entry.workingSet[pid].push_back(address);
    }
    
    //increase the working set
    concurrentMax += 1;
    
    //if concurrently is bigger than the max then replace our max record
    if(concurrentMax > max){
        max = concurrentMax;
    }
}

//handle the working set of inactive mems in main memory
void WS::removeWS(int pid){
    
    //remove all the mem address with this pid since it's done
    if(entry.workingSet.find(pid) == entry.workingSet.end()){
        concurrentMax -= entry.workingSet[pid].size();
        entry.workingSet[pid].clear();
    }
    
}

//print workingset
void WS::print(){
    //Working set: min size: 0; max size: 6.
    cout<<"Working set: "<<" min size: "<<this->min<<"; max size: "<<max<<"."<<endl;
}

//======================= Deactivate the process ====================
void WS::deactivateProcess(int pid,map<int,list<pair<int, int>>> *frame){
    /*
     check to see if we have the amount of frames in
     our main memory does not exceed the min free pool size
     if it does, then one or more processes need to be deactivated
     */
    for(auto p: entry.process){
        if(p.first != pid && frame->find(p.first) != frame->end()){
            p.second.active = false;

            //remove the frames from the mem regarding this process
            entry.currentPool -= frame->at(p.first).size();
            //check to see if we hold the threshold, else deactive more processes
            if(entry.memoryPageFrames - entry.currentPool > entry.minPool){
                return;
            }
        }
    }
}


//======================= Activatate back the process ====================
void WS::activateProcess(int pid, map<int,list<pair<int, int>>> *frame){
    
    int size = 0;
    if(frame->find(pid) != frame->end()){
        size = int(frame[pid].size());
    }
    
    //if process is already active then do nothing
    if(entry.process[pid].active == true && entry.memoryPageFrames - (entry.currentPool + size + 1) > entry.minPool){
        return;
    }

    /*
     check the min threshold if adding this process
     will lead to passing the min threshold then
     return without activating this process
     */
    if(entry.memoryPageFrames - (entry.currentPool + size + 1) <= entry.minPool){
        
        deactivateProcess(pid, frame);
    }
    
    //active this process
    entry.currentPool += size;
    entry.process[pid].active = true;
}
