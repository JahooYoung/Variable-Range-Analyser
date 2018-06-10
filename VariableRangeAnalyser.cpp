#include "VariableRangeAnalyser.h"
#include "SsaGraph.h"
#include <iostream>
using namespace std;

map<string, VariableRangeAnalyser*> func;

VariableRangeAnalyser::VariableRangeAnalyser(string code)
{
    SsaGraph sGraph(code, symtab, parameters);
    cout << "symtab: " << endl;
    // for (pair<const string, SymEntry> &i: symtab)
    //     i.second.first += "123";
    for (auto i: symtab)
        cout << i.second.first << " " << i.first << endl;
}

void VariableRangeAnalyser::ExecuteWithStdio()
{
    
}