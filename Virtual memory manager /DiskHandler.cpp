#include "DiskHandler.hpp"


//copy our address to hash table for faster search
DiskHandler::DiskHandler(const vector<MemoryAddress> &memoryAddress){
    for( auto ma: memoryAddress){
        //key is the binary value for each memory address
        this->memoryAddress[ma.getBinary()] = ma;
    }

}

//make a request;
void DiskHandler::makeRequest(string binary, int sid, int (*proc)[2]){
    pair<int, int> mem = {-1,-1};
   
    //if we couldn't find the address
    if(this->memoryAddress.find(binary) == this->memoryAddress.end()){
        
        close(*proc[0]); //close read descriptor
        write(*proc[1], &mem, sizeof(mem)); //write address value
        close(*proc[1]); //close write descriptor
        
        //signal sem its done
        struct sembuf sb; sb.sem_num = 0; sb.sem_op = 1; sb.sem_flg = 0; semop(sid, &sb, 1);
    }
    
    mem = memoryAddress[binary].getAddressValues();
    close(*proc[0]); //close read descriptor
    write(*proc[1], &mem, sizeof(mem)); //write address values
    close(*proc[1]); //close write descriptor
    
    //and signal the sem
    struct sembuf sb; sb.sem_num = 0; sb.sem_op = 1; sb.sem_flg = 0; semop(sid, &sb, 1);

}
