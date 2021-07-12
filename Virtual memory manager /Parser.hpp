#ifndef Parser_hpp
#define Parser_hpp

#include <stdio.h>
#include <queue>
#include <sstream>
#include "FIFO.hpp"
#include "LRU.hpp"
#include "MFU.hpp"
#include "OPT.hpp"
#include "WS.hpp"
#include "LRUX.hpp"
#include <iterator>

class Parser {
    Entry entry;
    void simulate();
public:
    Parser(const char argv[]);
};

#endif /* Parser_hpp */
