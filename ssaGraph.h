#ifndef SsaGraph_H
#define SsaGraph_H

#include "Utility.h"
#include <map>
#include <string>
#include <vector>

class Operand
{
public:
    enum {VAR, NUM} type;
    std::string var;
    double num;

    Operand();
    Operand(const std::string &_var);
    void Print();
};

class Statement
{
public:
    enum StType type;
    std::vector<Operand> operand;
    Interval *interval;
    bool visited;
    Statement *next[2];

    Statement();
    void Print();
    ~Statement();
};

class SsaGraph
{
private:
    std::string retValue;
public:
    Statement *entry;
    SsaGraph(std::string code, SymbolTable &symtab, std::vector<std::string> &parameters);
    std::vector<Statement*> Traverse(Statement *start, std::string end = "") const;
    void Transform(SymbolTable &symtab);
    ~SsaGraph();
};

#endif