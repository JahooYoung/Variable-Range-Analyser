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
    enum StType type; // constraint type
    Interval *interval; // for constant or constant interval
    std::vector<Variable*> var; // the list of variables that this constraint will use

    Constraint();
    Constraint(const Constraint &cst);
    Interval Evaluate();
    ~Constraint();
};

class Variable
{
public:
    std::vector<Variable*> outEdge; // the list of variables that can be affected by this variable
    Constraint cst; // the constraint that affects this variable
    Interval I; // the range of this variable
    bool visited; // for the use of traverse
    // for SCC Calculation
    int sccId;
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
    std::vector< std::vector<Variable*> > SCC; // Strong Connected Components
    void Tarjan(Variable *start, int &Timer, std::stack<Variable*> &stk);
public:
    ConstraintGraph();
    void BuildGraph(const SsaGraph &ssaGraph, SymbolTable &symtab);
    void BuildCopy(NodeVec &nodes, SymbolTable &symtab, SymbolTable &_symtab, std::string funcName);
    void Execute(std::map<Variable*, std::string> &varTab);
    ~ConstraintGraph();
};

#endif