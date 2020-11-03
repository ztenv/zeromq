#pragma once
#include <string>

#pragma push(pack,1)

struct snapshot{
    char instr[20];
    char exchange[10];
    double high;
    double open;
    double low;
    double close;
    double last;
};

#pragma pop()
