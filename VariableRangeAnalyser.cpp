#include "VariableRangeAnalyser.h"
#include "SsaGraph.h"
#include <iostream>
using namespace std;

map<string, VariableRangeAnalyser*> func;

VariableRangeAnalyser::VariableRangeAnalyser(string code, string name)
{
    funcName = name;
    SsaGraph ssaGraph(code, symtab, parameters);
    // cout << "symtab (before): " << endl;
    // for (auto i: symtab)
    //     cout << i.second.type << " " << i.first << endl;
    ssaGraph.Transform(symtab);
    // cout << "symtab (after): " << endl;
    // for (auto i: symtab)
    //     cout << i.second.type << " " << i.first << endl;
    constraintGraph.BuildGraph(ssaGraph, symtab);
}

NodeVec VariableRangeAnalyser::BuildCopy(NodeVec &nodes, SymbolTable &_symtab)
{
    // cout << "start copy" << endl;
    constraintGraph.BuildCopy(nodes, symtab, _symtab, funcName);
    // cout << "build copy ok" << endl;
    NodeVec params;
    for (auto param: parameters)
        params.push_back(symtab[param].node->newNode);
    return params;
}

void VariableRangeAnalyser::ExecuteWithStdio()
{
    // argc in [-inf, +inf]
    string retName = parameters.back();
    parameters.pop_back();
    for (auto param: parameters)
    {
        cout << param << " in ";
        cout.flush();
        string slow, shigh;
        cin >> slow >> shigh;
        double low, high;
        if (slow == "-inf") 
            low = -INF;
        else if (slow == "inf" || slow == "+inf")
            low = INF;
        else low = stod(slow);
        if (shigh == "-inf") 
            high = -INF;
        else if (shigh == "inf" || shigh == "+inf")
            high = INF;
        else high = stod(shigh);
        Variable *node = symtab[param].node;
        node->cst.type = ASN;
        node->cst.interval = new Interval(low, high);
    }
    for (auto var: symtab)
        varTab[var.second.node] = var.first;
    constraintGraph.Execute(varTab);
    cout << "result:" << endl;
    cout << retName << " in ";
    symtab[retName].node->I.Print();
    parameters.push_back(retName);
}