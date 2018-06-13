#ifndef ConstraintGraph_H
#define ConstraintGraph_H

#include "Utility.h"
#include "SsaGraph.h"
#include <vector>
#include <stack>
#include <map>
#include <string>

class Constraint
{
public:
    enum StType type;
    Interval *interval;
    std::vector<Variable*> var;

    Constraint();
    Constraint(const Constraint &cst);
    Interval Evaluate();
    ~Constraint();
};

class Variable
{
public:
    std::vector<Variable*> outEdge;
    Interval I;
    bool visited;
    int sccId;
    Constraint cst;
    // for SCC Calculation
    int state, dfn, low;
    // for Copy
    Variable *newNode;

    Variable();
    Variable(const Variable &var);
    bool Update(int stage);
};

typedef std::vector<Variable*> NodeVec;

class ConstraintGraph
{
private:
    std::vector< std::vector<Variable*> > SCC;
    void Tarjan(Variable *start, int &Timer, std::stack<Variable*> &stk);
public:
    ConstraintGraph();
    void BuildGraph(const SsaGraph &ssaGraph, SymbolTable &symtab);
    void BuildCopy(NodeVec &nodes, SymbolTable &symtab, SymbolTable &_symtab, std::string funcName);
    void Execute(std::map<Variable*, std::string> &varTab);
    ~ConstraintGraph();
};

#endif