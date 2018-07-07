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
    : type(cst.type), interval(NULL), var()
{
    if (cst.interval != NULL)
        interval = cst.interval->Copy();  // uncertain
}

Interval Constraint::Evaluate()
{
    // I2F, F2I, ASN, ADD, SUB, ISUB, MUL, DIV, IDIV, PHI, ITS
    Interval temp;
    switch (type)
    {
    case I2F:
        return var[0]->I;
    case F2I:
        return calc(var[0]->I, F2I);
    case ASN:
        if (interval != NULL)
            return *interval;
        else 
            return var[0]->I;
    case ADD: case SUB: case MUL: case DIV:
        if (interval == NULL)
            return calc(var[0]->I, var[1]->I, type);
        else 
            return calc(var[0]->I, *interval, type);
    case ISUB:
        return calc(*interval, var[0]->I, SUB);
    case IDIV:
        return calc(*interval, var[0]->I, DIV);
    case PHI:
        temp = var[0]->I;
        for (auto i: var)
            temp = calc(temp, i->I, type);
        return temp;
    case ITS:
        return calc(var[0]->I, *interval, ITS);
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

Variable::Variable(const Variable &var)
    : outEdge(), cst(var.cst), I(), visited(false), state(0), newNode(NULL) {}

bool Variable::Update(int stage)
{
    Interval oldI = I, e = cst.Evaluate();
    // cout << "      e:";
    // e.Print();
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
        if (cst.interval != NULL)
            cst.interval->ConvertToInterval();
        break;
    case 2:
        if (I.low == -INF && e.low > -INF) 
            I = Interval(e.low, I.high);
        else if (I.high == INF && e.high < INF)
            I = Interval(I.low, e.high);
        else if (I.low < e.low)
            I = Interval(e.low, I.high);
        else if (I.high > e.high)
            I = Interval(I.low, e.high);
        break;
    }
    if (oldI.undefined && I.undefined)
        return false;
    if (oldI.undefined || I.undefined)
        return true;
    if (oldI.empty && I.empty)
        return false;
    if (oldI.empty || I.empty)
        return true;
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
    if (u->dfn == u->low)
    {
        u->sccId = SCC.size();
        SCC.push_back(vector<Variable*>(1, u));
        for (; stk.top() != u; stk.pop())
        {
            Variable *v = stk.top();
            v->sccId = u->sccId;
            v->state = 2;
            SCC.back().push_back(v);
        }
        stk.pop();
        u->state = 2;
    }
}

void ConstraintGraph::BuildCopy(NodeVec &nodes, SymbolTable &symtab, SymbolTable &_symtab, string funcName)
{
    static int timer = 0;
    timer++;
    for (auto var: symtab)
    {
        var.second.node->newNode = new Variable(*var.second.node);
        nodes.push_back(var.second.node->newNode);
        _symtab[var.first + "__" + funcName + "_" + to_string(timer)] = 
            SymEntry(var.second.type, var.second.node->newNode);
    }
    // for (auto scc: SCC)
    //     for (auto node: scc)
    //     {
    //         node->newNode = new Variable(*node);
    //         cout << "aaa" << endl;
    //         nodes.push_back(node->newNode);
    //     }
    for (auto scc: SCC)
        for (auto node: scc)
        {
            for (auto v: node->outEdge)
                node->newNode->outEdge.push_back(v->newNode);
            for (auto v: node->cst.var)
                node->newNode->cst.var.push_back(v->newNode);
        }
    
}

void ConstraintGraph::BuildGraph(const SsaGraph &ssaGraph, SymbolTable &symtab)
{
    // cout << "Begin to build constraint graph" << endl;
    vector<Variable*> nodes;
    map<Variable*, string> varTab;
    for (auto &var: symtab)
    {
        var.second.node = new Variable();
        nodes.push_back(var.second.node);
    }
    for (auto stm: ssaGraph.Traverse(ssaGraph.entry))
    {
        if (stm->type > CAL) 
            continue;
        // stm->Print();
        Variable *var = NULL;
        for (auto op: stm->operand)
        {
            if (var == NULL)
                var = symtab[op.var].node;
            else if (stm->type != CAL && op.type == Operand::VAR && symtab.count(op.var))
            {
                // cout << op.var << endl;
                Variable *v = symtab[op.var].node;
                var->cst.var.push_back(v);
                v->outEdge.push_back(var);
            }
        }
        var->cst.type = stm->type;
        var->cst.interval = stm->interval;
        if (stm->interval != NULL)
            var->cst.interval->ConnectToVariable(symtab, var);
        // I2F, F2I, ASN, ADD, SUB, MUL, DIV, PHI, ITS, CAL
        if (stm->type == ASN)
        {
            if (stm->operand[1].type == Operand::NUM)
                var->cst.interval = new Interval(stm->operand[1].num, stm->operand[1].num);
        }
        else if (stm->type >= ADD && stm->type <= DIV)
        {
            if (stm->operand[2].type == Operand::NUM)
                var->cst.interval = new Interval(stm->operand[2].num, stm->operand[2].num);
            else if (stm->operand[1].type == Operand::NUM)
            {
                if (stm->type == SUB) 
                    var->cst.type = ISUB;
                else 
                    var->cst.type = IDIV;
                var->cst.interval = new Interval(stm->operand[1].num, stm->operand[1].num);
            }
        }
        else if (stm->type == CAL) // inline
        {
            vector<Variable*> params = func[stm->operand[1].var]->BuildCopy(nodes, symtab);
            // cout << "copy ok" << endl;
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
                    Variable *cnode = symtab[stm->operand[i].var].node;
                    node->cst.var.push_back(cnode);
                    cnode->outEdge.push_back(node);
                }
                else
                    node->cst.interval = new Interval(stm->operand[i].num, stm->operand[i].num);
                i++;
            }
        }
    }
    for (auto var: symtab)
        varTab[var.second.node] = var.first;
    // cout << "Build constraint graph complete" << endl;

    // Calculate SCC
    int Timer = 0;
    stack<Variable*> stk;
    for (auto node: nodes)
        if (node->state == 0)
            Tarjan(node, Timer, stk);
    // int totalSize = nodes.size();
    // for (auto scc: SCC)
    // {
    //     // totalSize -= scc.size();
    //     cout << "SCC " << scc[0]->sccId << " has :" << endl;
    //     for (auto node: scc)
    //         cout << "  " << varTab[node] << endl;
    // }
    // cout << "totalSize: " << totalSize << endl;
    // Topological Sort
    vector<int> degree(SCC.size(), 0);
    // for (auto node: nodes)
    // {
    //     cout << varTab[node] << " sccId = " << node->sccId << ", controls: " << endl;
    //     for (auto v: node->outEdge)
    //         cout << "  " << varTab[v] << endl;
    // }
    for (auto node: nodes)
        for (auto v: node->outEdge)
            if (node->sccId != v->sccId)
            {
                // cout << node->sccId << ' ' << v->sccId << endl;
                degree[v->sccId]++;
            }
    // for (auto d: degree)
    //     cout << d << " ";
    // cout << endl;
    queue<int> que;
    for (size_t i = 0; i < degree.size(); i++)
        if (degree[i] == 0) 
            que.push(i);
    vector< vector<Variable*> > tempSCC;
    int sccId = 0;
    for (; !que.empty(); que.pop())
    {
        // cout << que.front() << ": ";
        for (auto node: SCC[que.front()])
            for (auto v: node->outEdge)
                if (node->sccId != v->sccId)
                {
                    degree[v->sccId]--;
                    if (degree[v->sccId] == 0)
                        que.push(v->sccId);
                }
        // for (auto d: degree)
        //     cout << d << " ";
        // cout << endl;
        for (auto node: SCC[que.front()])
            node->sccId = sccId;
        sccId++;
        tempSCC.push_back(SCC[que.front()]);
    }
    // cout << endl;
    SCC = tempSCC;
    // for (auto scc: SCC)
    // {
    //     cout << "SCC " << scc[0]->sccId << " has :" << endl;
    //     for (auto node: scc)
    //         cout << "  " << varTab[node] << endl;
    // }
    // cout << "Build SCC complete" << endl;
}

void ConstraintGraph::Execute(std::map<Variable*, std::string> &varTab)
{
    // cout << "Begin to Execute CG" << endl;
    for (auto scc: SCC)
    {
        int sccId = scc[0]->sccId;
        // cout << "Begin to Execute SSC " << sccId
        //      << ", size " << scc.size() << endl;
        for (auto node: scc)
            node->I = Interval();
        for (int stage = 0; stage < 3; stage++)
        {
            // cout << "  Stage " << stage << endl;
            queue<Variable*> que;
            for (auto node: scc)
            {
                que.push(node);
                node->visited = true;
            }
            for (; !que.empty(); que.pop())
            {
                Variable *u = que.front();
                // cout << "    now: " << varTab[u] << " ";
                if (u->Update(stage))
                    for (auto v: u->outEdge)
                        if (v->sccId == sccId && !v->visited)
                        {
                            v->visited = true;
                            que.push(v);
                        }
                u->visited = false;
                // cout << "        ";
                // u->I.Print();
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