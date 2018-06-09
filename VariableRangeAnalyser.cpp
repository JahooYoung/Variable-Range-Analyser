#include "VariableRangeAnalyser.h"
#include "ssaGraph.h"
#include <iostream>
using namespace std;

map<string, VariableRangeAnalyser*> func;

VariableRangeAnalyser::VariableRangeAnalyser(string code)
{
    SsaGraph sGraph(code);
}

void VariableRangeAnalyser::ExecuteWithStdio()
{
    
}