#ifndef ssaGraph_H
#define ssaGraph_H

#include <map>
#include <string>

class Operand
{
public:
    enum {VAR, INT, FLOAT} type;
    union 
    {
        std::string var;
        int intNum;
        double floatNum;
    };
};

class Statement
{
public:
    enum {I2F, F2I, ASN, ADD, SUB, MUL, DIV, PHI, LES, LEQ, GTR, GEQ, EQU, NEQ, CAL} type;
    Operand opd[3];
    Statement *next[2];
};

class SsaGraph
{
private:
    Statement *entry;
    std::string retValue;
public:
    SsaGraph(std::string code);
    ~SsaGraph();
};

#endif