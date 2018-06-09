#ifndef VariableRangeAnalyser_H
#define VariableRangeAnalyser_H

#include <map>
#include <string>

class VariableRangeAnalyser
{
private:

public:
    VariableRangeAnalyser(std::string code);
    void ExecuteWithStdio();
    void Execute();
};

extern std::map<std::string, VariableRangeAnalyser*> func;

#endif