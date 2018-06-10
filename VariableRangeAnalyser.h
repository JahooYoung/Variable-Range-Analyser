#ifndef VariableRangeAnalyser_H
#define VariableRangeAnalyser_H

#include "ConstraintGraph.h"
#include <map>
#include <string>
#include <vector>

typedef std::pair<std::string, Node*> SymEntry;
typedef std::map<std::string, SymEntry> SymbolTable;

class VariableRangeAnalyser
{
private:
    SymbolTable symtab;
    std::vector<std::string> parameters;  // The last one is the return value
public:
    VariableRangeAnalyser(std::string code);
    void ExecuteWithStdio();
    void Execute();
};

extern std::map<std::string, VariableRangeAnalyser*> func;

#endif