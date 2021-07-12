#include "LRUX.hpp"


//Copy the process entry and init the LRUX
LRUX::LRUX(const Entry &entry){
    this->entry = entry;
    
    //copy the memoryaddress into disk for I/O op
    disk = DiskHandler(entry.memoryAddress);
    cout<<"LRUX: "<<endl;
    //init our LRUX algo
    init();
};



//init our LRUX algorithm
void LRUX::init(){
    
    //init semaphore for disk request
    int segmet(key_t key, int nsems, int semflag);
    static key_t key;
    int sid = semget(key, 1, 0666 | IPC_CREAT);
    
    //create semaphore for page replacement and process activation
    int b1 = semget(1525557, 1, 0666 | IPC_CREAT);
    int b2 = semget(15255572, 1, 0666 | IPC_CREAT);
    
    struct sembuf sb; sb.sem_num = 0; sb.sem_flg = 0;
    
    //frame for segment#, page# in main memory
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
        
        //get the segment number and page number
        pair<int,int> address = entry.memoryAddress[memoryIndex].getAddressValues();
        
        //if memory address is -1, go to next process
        if(entry.memoryAddress[memoryIndex].getBinary() == "-1"){
                        
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
            
            //check for this address in our frame
            bool inFrame = false;
            list<pair<int, int>> copyFrame = frame[pid];
            
            //look for the address in our frame
            while(copyFrame.empty() == false){
                if(copyFrame.front() == address){
                    inFrame = true;
                    break;
                }
                copyFrame.pop_front();
            }
            
            if (inFrame){
                //increase memory index
                memoryIndex += 1;
                
                //go to loop
                goto Loop;
            }
            
            //if the size of our queue is less than the given frame
            //go ahead add the memory to it
            if(frame[pid].size() < entry.numberOfPageFrames){
                
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
                    frame[pid].push_front(address);
                    
                    //increase page fault
                    entry.process[pid].pageFault += 1;
                    
                    int returnStatus;
                    waitpid(fid, &returnStatus, 0);
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
                    
                    //if no more space then do pagereplacement
                    pair<int, int> victim = findLRUX(memoryIndex, frame);
                    frame[pid].remove(victim);
                    
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
                        frame[pid].push_front(address);
                        
                        close(proc[0]); //close the read descriptor
                        
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
                
                frame[pid].push_front(address);
                
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
}



//======================= Handle Algorithm Request ====================
//find the LRUX mem address for the current process within the frame
pair<int,int> LRUX::findLRUX(int index, map<int,list<pair<int, int>>> frame){
    if(index <= 0){
        return entry.memoryAddress[index].getAddressValues();
    }
    
    //create a table of mem address and times they came up in the past
    map<pair<int,int>,int> table;
    //max distance is the one that should be choosen as victim
    pair<int,int> max = entry.memoryAddress[index-1].getAddressValues();
    int j = 0;
    
    //get the pid for the current mem address requesting a page change
    int pid = entry.memoryAddress[index].getPid();
    
    //iterate through the currently mem addresses in the main memory for this process
    for(auto p: frame[pid]){
        
        //assign the lookahead window
        int x = entry.lookAhead;
        
        for(int i = index - 1;i > -1;i--){
            if(x == 0){
                break;
            }
            
            //go through the mem address in our frames and find out where they were used last time
            if(entry.memoryAddress[i].getPid() == pid && p == entry.memoryAddress[i].getAddressValues() && entry.memoryAddress[i].getBinary() != "-1"){
                
                /*
                 index - i would give us the distance between this address index
                 from the current index
                 */
                if(table.find(p) == table.end()){
                    table[p] = index - i;
                }
                            
                //max distance from current index
                if(table[p] > j){
                    j = table[p];
                    max = p;
                }
                
                //decrease x
                x -= 1;
            }
            
        }
    }

    return max;
}


//======================= Deactivate the process ====================
void LRUX::deactivateProcess(int pid,map<int,list<pair<int, int>>> *frame){
    
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
void LRUX::activateProcess(int pid, map<int,list<pair<int, int>>> *frame){
    
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
