#include "VariableRangeAnalyser.h"
#include <iostream>
using namespace std;

map<string, VariableRangeAnalyser*> func;

VariableRangeAnalyser::VariableRangeAnalyser(string code)
    : symtab(), parameters(), ssaGraph(code, symtab, parameters)
{
    cout << "symtab (before): " << endl;
    for (auto i: symtab)
        cout << i.second.type << " " << i.first << endl;
    ssaGraph.Transform(symtab);
    cout << "symtab (after): " << endl;
    for (auto i: symtab)
        cout << i.second.type << " " << i.first << endl;
}

void VariableRangeAnalyser::ExecuteWithStdio()
{
    
}