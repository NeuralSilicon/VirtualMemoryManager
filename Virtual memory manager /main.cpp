#include <iostream>
#include "Parser.hpp"

int main(int argc, const char * argv[]) {
    
    if (argc > 0){
        Parser parser = Parser(argv[1]);
    }
    
    return 0;
}
