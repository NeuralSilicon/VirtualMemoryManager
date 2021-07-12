#include "Parser.hpp"
#include <cstring>

Parser::Parser(const char argv[]){
    
    int size = int(std::strlen(argv)); //size of our argument
    string str; //holds our entire argument
    
    if (size == 0 ){ //Stop if no argument was passed
        return;
    }
    
    for (int i = 0; i<size;i++){ //loop through the argv
        str += argv[i];
    }
  
    vector<string> lines; //hold the strings
    std::stringstream iss(str);
    
    //parsing the string line per line to extract the info
    while(iss.good())
    {
        std::string line;
        getline(iss,line,'\n');
        if (line != " " && line != ""){ //if line is not empty
            for(int i = int(line.length()) - 1 ; i>0 ; i--){ //remove the space at the end of line
                if(line[i] == ' '){
                    line = line.substr(0,i-1);
                }else{
                    break;
                }
            }
            lines.push_back(line);
        }
    }
    
    //======================= parse first 8 line ====================
    
    for(int i = 0; i < 8; i++){
        std::istringstream stream(lines[i]);
        std::string str1;
        stream >> str1;
        entry.insertValues(i, str1);
    }

    //======================= parse processes ====================
    
    for(int i = 8; i < 8 + entry.numberOfProcess ; i++){
        
        //split by space
        std::istringstream stream(lines[i]);
        std::string str1;
        std::string str2;
        stream >> str1 >> str2;
        
        Process process = Process();
        process.pid = stoi(str1);
        process.size = stoi(str2);
        entry.process.insert(pair<int, Process>(process.pid,process));

    }
    
    //======================= parse memory address ====================
    
    for (int i = 8 + entry.numberOfProcess; i < lines.size(); i++) {
        
        //split by space
        std::istringstream stream(lines[i]);
        std::string str1;
        std::string str2;
        stream >> str1 >> str2;
        
        struct MemoryAddress memAddress;
        
        if(str2 == "-1"){
            memAddress.setBinary(str2, stoi(str1));
            memAddress.extractValues(entry.maxSegmentSize, entry.pageSize);
        }else{
            //remove the B at the end
            memAddress.setBinary(str2.substr(0, str2.size() - 1), stoi(str1));
            memAddress.extractValues(entry.maxSegmentSize, entry.pageSize);
        }
        entry.memoryAddress.push_back(memAddress);
    }
    
    simulate();
}

//======================= simulate the replacement algos ====================
void Parser::simulate(){

    FIFO fifo = FIFO(entry);
    cout<<endl;
    LRU lru = LRU(entry);
    cout<<endl;
    LRUX lrux = LRUX(entry);
    cout<<endl;
    MFU mfu = MFU(entry);
    cout<<endl;
    OPT opt = OPT(entry);
    cout<<endl;
    WS ws = WS(entry);
    cout<<endl;
}
