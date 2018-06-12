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
    Statement *entry;
    std::string retValue;
public:
    SsaGraph(std::string code, SymbolTable &symtab, std::vector<std::string> &parameters);
    vector<Statement*> Traverse(Statement *start = this->entry, string end = "") const;
    void Transform(SymbolTable &symtab);
    ~SsaGraph();
};

#endif