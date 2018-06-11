#ifndef VariableRangeAnalyser_H
#define VariableRangeAnalyser_H

#include "Utility.h"
#include "ConstraintGraph.h"
#include "SsaGraph.h"
#include <map>
#include <string>
#include <vector>

class VariableRangeAnalyser
{
private:
    SymbolTable symtab;
    std::vector<std::string> parameters;  // The last one is the return value
    SsaGraph ssaGraph;
public:
    VariableRangeAnalyser(std::string code);
    void ExecuteWithStdio();
    void Execute();
};

extern std::map<std::string, VariableRangeAnalyser*> func;

#endif