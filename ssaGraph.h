#ifndef SsaGraph_H
#define SsaGraph_H

#include "VariableRangeAnalyser.h"
#include <map>
#include <string>
#include <vector>

class Operand
{
public:
    enum {VAR, INT, FLOAT} type;
    std::string var;
    int intNum;
    double floatNum;

    // Operand();
    void Print();
};

class Statement
{
public:
    enum {I2F, F2I, ASN, ADD, SUB, MUL, DIV, PHI, CAL, LES, LEQ, GTR, GEQ, EQU, NEQ, NOP} type;
    std::vector<Operand> operand;
    Statement *next[2];
    bool visited;

    Statement();
    void Print();
};

class SsaGraph
{
private:
    Statement *entry;
    std::string retValue;
public:
    SsaGraph(std::string code, SymbolTable &symtab, std::vector<std::string> &parameters);
    ~SsaGraph();
};

#endif