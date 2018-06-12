#include "ConstraintGraph.h"
#include "VariableRangeAnalyser.h"
#include <algorithm>
#include <queue>
using namespace std;

/**
 * Constraint Block
 */

Constraint::Constraint() 
    : type(NOP), interval(NULL), var() {}

Constraint::Constraint(const Constraint &cst)
    : type(cst.type), interval(NULL), var(cst.var)
{
    interval = cst.interval->Copy();  // uncertain
}

Interval Constraint::Evaluate()
{
    // I2F, F2I, ASN, ADD, SUB, ISUB, MUL, DIV, IDIV, PHI, ITS
    Interval temp;
    switch (type)
    {
    case F2I:
        return calc(var[0]->I, F2I);
    case ASN:
        return var[0]->I;
    case ADD: case SUB: case MUL: case DIV:
        if (interval == NULL)
            return calc(var[0]->I, var[1]->I);
        else 
            return calc(var[0]->I, *interval);
    case ISUB:
        return calc(*interval, SUB);
    case IDIV:
        return calc(*interval, DIV);
    case PHI: case ITS:
        temp = var[0]->I;
        for (auto i: var)
            temp = calc(temp, i->I, type);
        return temp;
    default:
        return Interval();
    }
}

Constraint::~Constraint()
{
    if (interval != NULL)
        delete interval;
}

/**
 * Variable Block
 */

Variable::Variable()
    : visited(false), state(0) {}

bool Variable::Update(int stage)
{
    Interval oldI = I, e = cst.Evaluate();
    switch (stage)
    {
    case 0:
        if (I.undefined) 
            I = e;
        else if (e.low < I.low && e.high > I.high) 
            I = Interval(-INF, +INF);
        else if (e.low < I.low)
            I = Interval(-INF, I.high);
        else if (e.high > I.high)
            I = Interval(I.low, +INF);
        break;
    case 1:
        cst.interval->ConvertToInterval();
        break;
    case 2:
        if (I.low == -INF && e.low > -INF) 
            I = Interval(e.low, I.high);
        else if (I.high == INF && e.high < INF)
            I = Interval(I.low, e.high);
        else if (I.low > e.low)
            I = Interval(e.low, I.high);
        else if (I.high < e.high)
            I = Interval(I.low, e.high);
        break;
    }
    if (I.low == oldI.low && I.high == oldI.high)
        return false;
    return true;
}

/**
 * ConstraintGraph Block
 */

ConstraintGraph::ConstraintGraph() {}

void ConstraintGraph::Tarjan(Variable *u, int &Timer, stack<Variable*> &stk)
{
    u->state = 1;
    u->dfn = u->low = ++Timer;
    stk.push(u);
    for (auto v: u->outEdge)
    {
        if (v->state == 2) continue;
        if (v->state == 0)
        {
            Tarjan(v, Timer, stk);
            u->low = min(u->low, v->low);
        }
        else
            u->low = min(u->low, v->dfn);
    }
    u->sccId = SCC.size();
    SCC.push_back(vector<Variable*>(1, u));
    for (; stk.top() != u; stk.pop())
    {
        Variable *v = stk.top();
        v->sccId = u->sccId;
        SCC.back().push_back(v);
    }
}

void ConstraintGraph::BuildCopy(NodeVec &nodes)
{
    for (auto scc: SCC)
        for (auto node: scc)
        {
            node->newNode = new Variable(*node);
            nodes.push_back(node->newNode);
        }
    for (auto scc: SCC)
        for (auto node: scc)
        {
            for (auto v: node->outEdge)
                node->newNode->outEdge.push_back(v->newNode);
            for (auto v: node->cst.var)
                node->newNode->cst.var.push_back(v->newNode);
        }
    
}

ConstraintGraph::ConstraintGraph(const SsaGraph &ssaGraph, SymbolTable &symtab)
{
    vector<Variable*> nodes;
    for (auto &var: symtab)
    {
        var.second.node = new Variable();
        nodes.push_back(var.second.node);
    }
    for (auto stm: ssaGraph.Traverse())
    {
        if (stm->type > CAL) 
            continue;
        Variable *var = NULL;
        for (auto op: stm->operand)
        {
            if (var == NULL)
                var = symtab[op.var].node;
            else if (op.type == Operand::VAR)
            {
                var->cst.var.push_back(symtab[op.var].node);
                symtab[op.var].node->outEdge.push_back(var);
            }
        }
        var->cst.type = stm->type;
        var->cst.interval = stm->interval;
        var->cst.interval->ConnectToVariable(symtab);
        // I2F, F2I, ASN, ADD, SUB, MUL, DIV, PHI, ITS, CAL
        if (stm->type >= ASN && stm->type <= DIV)
        {
            if (stm->operand[1].type == Operand::NUM)
                var->cst.interval = new Interval(stm->operand[1].num, stm->operand[1].num);
            else if (stm->operand[0].type == Operand::NUM)
            {
                if (stm->type == SUB) 
                    var->cst.type = ISUB;
                else 
                    var->cst.type = IDIV;
                var->cst.interval = new Interval(stm->operand[0].num, stm->operand[0].num);
            }
        }
        else if (stm->type == CAL) // inline
        {
            vector<Variable*> params = func[stm->operand[1].var]->BuildCopy(nodes);
            var->cst.type = ASN;
            var->cst.var.push_back(params.back());
            params.back()->outEdge.push_back(var);
            params.pop_back();
            int i = 2;
            for (auto node: params)
            {
                node->cst.type = ASN;
                if (stm->operand[i].type == Operand::VAR)
                {
                    node->cst.var.push_back(var);
                    var->outEdge.push_back(node);
                }
                else
                    node->cst.interval = new Interval(stm->operand[i].num, stm->operand[i].num);
                i++;
            }
        }
    }
    // parameters ?
    // Calculate SCC
    int Timer = 0;
    stack<Variable*> stk;
    for (auto node: nodes)
        if (node->state == 0)
            Tarjan(node, Timer, stk);
}

void ConstraintGraph::Execute()
{
    for (auto scc: SCC)
    {
        int sccId = scc[0]->sccId;
        for (auto node: scc)
            node->I = Interval();
        for (int stage = 0; stage < 3; stage++)
        {
            queue<Variable*> que;
            for (auto node: scc)
            {
                que.push(node);
                node->visited = true;
            }
            for (; !que.empty(); que.pop())
            {
                Variable *u = que.front();
                if (u->Update(stage))
                    for (auto v: u->outEdge)
                        if (v->sccId == sccId && !v->visited)
                        {
                            v->visited = true;
                            que.push(v);
                        }
                u->visited = false;
            }
        }
    }
}

ConstraintGraph::~ConstraintGraph()
{
    for (auto scc: SCC)
        for (auto node: scc)
            delete node;
}