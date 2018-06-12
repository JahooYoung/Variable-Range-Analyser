#include "SsaGraph.h"
#include <vector>
#include <sstream>
#include <regex>
#include <iostream>
using namespace std;

Operand::Operand() {}

Operand::Operand(const string &_var)
    : type(VAR), var(_var) {}

Statement::Statement() : type(NOP), interval(NULL), visited(false)
{
    next[0] = next[1] = NULL;
}

Statement::~Statement()
{
    // if (interval != NULL)
    //     delete interval;
}

void Operand::Print()
{
    switch (type)
    {
    case VAR:
        cout << var << ' ';
        break;
    case NUM:
        cout << num << ' ';
        break;
    }
}

void Statement::Print()
{
    cout << type << ' ';
    for (Operand i: operand)
        i.Print();
    cout << endl;
}

class TokenStream
{
private:
    vector<string> tokens;
    vector<string>::iterator pt;
    static regex symbols;
public:
    TokenStream(string s)
    {
        s = regex_replace(s, symbols, " $0 ");
        istringstream iss(s);
        while (iss >> s) 
            tokens.push_back(s);
        pt = tokens.begin();
    }
    void Append(string s)
    {
        s = regex_replace(s, symbols, " $0 ");
        istringstream iss(s);
        size_t p = pt - tokens.begin();
        while (iss >> s)
        {
            tokens.push_back(s);
            // cout << s << ":::";
        }
        // cout << endl;
        pt = tokens.begin() + p;
    }
    const string &Now()
    {
        return *pt;
    }
    const string &Peek(const int &n)
    {
        return *(pt + n);
    } 
    void Forward(int n = 1)
    {
        pt += n;
    }
};

regex TokenStream::symbols = regex("[\\(\\)<>,;=]");

Operand FetchOperand(TokenStream &tokens)
{
    Operand operand;
    if ('0' <= tokens.Now()[0] && tokens.Now()[0] <= '9')
    {
        operand.type = Operand::NUM;
        operand.num = stod(tokens.Now());
        tokens.Forward();
    }
    else
    {
        operand.type = Operand::VAR;
        operand.var = tokens.Now();
        tokens.Forward();
        if (tokens.Now() == "(" && 
            (tokens.Peek(1) == "D" || ('0' <= tokens.Peek(1)[0] && tokens.Peek(1)[0] <= '9')))
        {
            if (tokens.Peek(1) == "D")
                operand.var = regex_replace(operand.var, regex("^(\\w+)_\\d+$"), "$1");
            tokens.Forward(3);
            if (tokens.Now() == "(") 
                tokens.Forward(3);
        }
    }
    return operand;
}

int ParseGoto(TokenStream &tokens)
{
    int bbId = stoi(tokens.Peek(3));
    while (tokens.Now() != ";")
        tokens.Forward();
    tokens.Forward();
    return bbId;
}

Statement* ParseIf(TokenStream &tokens, map<int, Statement*> &bbAddr, 
    map<int, vector<Statement**> > &ukBbAddr)
{
    tokens.Forward(2);
    Statement *stm = new Statement();
    stm->operand.push_back(FetchOperand(tokens));
    if (tokens.Now() == "<" || tokens.Now() == ">")
    {
        if (tokens.Peek(1) == "=") 
            stm->type = tokens.Now() == "<" ? LEQ : GEQ;
        else
            stm->type = tokens.Now() == "<" ? LES : GTR;
    }
    else if (tokens.Now() == "!")
        stm->type = NEQ;
    else 
        stm->type = EQU;
    if (tokens.Peek(1) == "=")
        tokens.Forward(2);
    else
        tokens.Forward(1);
    stm->operand.push_back(FetchOperand(tokens));
    tokens.Forward(); // if (i_1 < j_1)
    // cout << "111" << endl;
    int bbId = ParseGoto(tokens);
    // cout << "333" << endl;
    if (bbAddr.count(bbId)) 
        stm->next[0] = bbAddr[bbId];
    else if (ukBbAddr.count(bbId))
        ukBbAddr[bbId].push_back(&stm->next[0]);
    else 
        ukBbAddr[bbId] = vector<Statement**> (1, &stm->next[0]);
    if (tokens.Now() != "else")
        return stm;
    tokens.Forward(); // else
    // cout << "222" << endl;
    bbId = ParseGoto(tokens);
    if (bbAddr.count(bbId)) 
        stm->next[1] = bbAddr[bbId];
    else if (ukBbAddr.count(bbId))
        ukBbAddr[bbId].push_back(&stm->next[1]);
    else 
        ukBbAddr[bbId] = vector<Statement**> (1, &stm->next[1]);
    return stm;
}

Statement* ParseAssignment(TokenStream &tokens, SymbolTable &symtab)
{
    Statement *stm = new Statement();
    if (tokens.Now() == "#")
    {
        tokens.Forward();
        stm->operand.push_back(FetchOperand(tokens));
        string var = stm->operand[0].var;
        if (!symtab.count(var)) 
        {
            SymEntry::Type type = symtab[regex_replace(var, regex("^(\\w+)_\\d+$"), "$1")].type;
            symtab[var] = SymEntry(type);
        }
        for (tokens.Forward(2); tokens.Now() != ">"; )
        {
            tokens.Forward();
            stm->operand.push_back(FetchOperand(tokens));
        }
        stm->type = PHI;
        return stm;
    }
    else
    {
        stm->operand.push_back(FetchOperand(tokens));
        string var = stm->operand[0].var;
        if (!symtab.count(var)) 
        {
            SymEntry::Type type = symtab[regex_replace(var, regex("^(\\w+)_\\d+$"), "$1")].type;
            symtab[var] = SymEntry(type);
        }
        tokens.Forward();
        if (tokens.Now() == "(")
        {
            tokens.Forward();
            if (tokens.Now() == "int")
                stm->type = F2I;
            else
                stm->type = I2F;
            tokens.Forward(2);
            stm->operand.push_back(FetchOperand(tokens));
            return stm;
        }
        stm->operand.push_back(FetchOperand(tokens));
            // cout << "aaa:" << tokens.Now() << endl;
        if (tokens.Now() == ";")
        {
            stm->type = ASN;
            return stm;
        }
        if (tokens.Now() == "(")
        {
            stm->type = CAL;
            while (tokens.Now() != ")")
            {
                tokens.Forward();
                cout << tokens.Now() << endl;
                stm->operand.push_back(FetchOperand(tokens));
            }
            return stm;
        }
        if (tokens.Now() == "+")
            stm->type = ADD;
        else if (tokens.Now() == "-")
            stm->type = SUB;
        else if (tokens.Now() == "*")
            stm->type = MUL;
        else if (tokens.Now() == "/")
            stm->type = DIV;
        tokens.Forward();
        stm->operand.push_back(FetchOperand(tokens));
        return stm;
    }
}

SsaGraph::SsaGraph(string code, SymbolTable &symtab, vector<string> &parameters)
{
    istringstream lines(code);
    regex spaces("^\\s*\\{*\\}*$");
    map<int, Statement*> bbAddr;
    map<int, vector<Statement**> > ukBbAddr;
    Statement *lastStm = NULL;
    int bbStart = -1;
    bool parseHead = false;
    entry = NULL;
    for (string line; getline(lines, line); )
    {
        if (regex_match(line, spaces)) continue;
        // cout << line << endl;
        TokenStream tokens(line);
        Statement *stm;
        if (tokens.Now() == "if") 
        {
            for (int i = 0; i < 3; i++)
            {
                getline(lines, line);
                tokens.Append(line);
            }
            stm = ParseIf(tokens, bbAddr, ukBbAddr);
            if (lastStm != NULL)
                lastStm->next[0] = stm;
            lastStm = NULL;
        } 
        else if (tokens.Now() == "goto")
        {
            int bbId = ParseGoto(tokens);
            // cout << bbId << endl;
            if (lastStm != NULL)
            {
                if (bbAddr.count(bbId)) 
                    lastStm->next[0] = bbAddr[bbId];
                else if (ukBbAddr.count(bbId))
                    ukBbAddr[bbId].push_back(&lastStm->next[0]);
                else 
                    ukBbAddr[bbId] = vector<Statement**> (1, &lastStm->next[0]);
                lastStm = NULL;
                continue;
            }
            else
            {
                stm = new Statement();
                stm->type = NOP;
                if (bbAddr.count(bbId)) 
                    stm->next[0] = bbAddr[bbId];
                else if (ukBbAddr.count(bbId))
                    ukBbAddr[bbId].push_back(&stm->next[0]);
                else 
                    ukBbAddr[bbId] = vector<Statement**> (1, &stm->next[0]);
            }
        }
        else if (tokens.Now() == "<" && tokens.Peek(1) == "bb")
        {
            bbStart = stoi(tokens.Peek(2));
            continue;
        }
        else if (tokens.Now() == "int" || tokens.Now() == "float")
        {
            symtab[tokens.Peek(1)] = SymEntry(tokens.Now());
            continue;
        }
        else if (!parseHead && tokens.Peek(1) == "(")
        {
            parseHead = true;
            if (tokens.Peek(2) == ")") 
                continue;
            for (tokens.Forward(); tokens.Now() != ")"; tokens.Forward(2))
            {
                tokens.Forward();
                symtab[tokens.Peek(1)] = SymEntry(tokens.Now());
                parameters.push_back(tokens.Peek(1));
            }
            continue;
        }
        else if (tokens.Now() == "<" && tokens.Peek(1)[0] == 'L')
        {
            continue;
        }
        else if (tokens.Now() == "return")
        {
            parameters.push_back(tokens.Peek(1));
            continue;
        }
        else if (tokens.Now() == "#" || tokens.Peek(1) == "=")
        {
            stm = ParseAssignment(tokens, symtab);
            if (lastStm != NULL)
                lastStm->next[0] = stm;
            lastStm = stm;
        }
        else continue;
        stm->Print();
        if (entry == NULL)
            entry = stm;
        if (bbStart != -1)
        {
            bbAddr[bbStart] = stm;
            if (ukBbAddr.count(bbStart))
            {
                for (Statement** i: ukBbAddr[bbStart])
                    *i = stm;
                ukBbAddr.erase(bbStart);
            }
            bbStart = -1;
        }
    }
}

vector<Statement*> SsaGraph::Traverse(Statement *start, string end) const
{
    vector<Statement*> que;
    if (entry == NULL) 
        return que;
    que.push_back(entry);
    entry->visited = true;
    for (size_t i = 0; i < que.size(); i++)
    {
        Statement *u = que[i];
        for (int j = 0; j < 2; j++)
        {
            Statement *v = u->next[j];
            if (v != NULL && !v->visited && !(v->type <= CAL && v->operand[0].var == end))
            {
                que.push_back(v);
                v->visited = true;
            }
        }
    }
    for (auto i: que)
        i->visited = false;
    return que;
}

void SsaGraph::Transform(SymbolTable &symtab)
{
    cout << "Transform:" << endl;
    for (auto stm: Traverse())
    {
        if (!(stm->type >= LES && stm->type <= NEQ)) 
            continue;
        stm->Print();
        Interval *interval[2][2] = {NULL};
        double delta = symtab[stm->operand[0].var].type == SymEntry::INT ? 1 : 0;
        switch (stm->type)
        {
        case LES:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[0][0] = new FutureInterval("", -INF, stm->operand[1].var, -delta);
                interval[1][0] = new FutureInterval(stm->operand[0].var, delta, "", INF);
                interval[0][1] = new FutureInterval(stm->operand[1].var, 0, "", INF);
                interval[1][1] = new FutureInterval("", -INF, stm->operand[0].var, 0);
            }
            else
            {
                interval[0][0] = new Interval(-INF, stm->operand[1].num - delta);
                interval[0][1] = new Interval(stm->operand[1].num, INF);
            }
            break;
        case LEQ:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[0][0] = new FutureInterval("", -INF, stm->operand[1].var, 0);
                interval[1][0] = new FutureInterval(stm->operand[0].var, 0, "", INF);
                interval[0][1] = new FutureInterval(stm->operand[1].var, delta, "", INF);
                interval[1][1] = new FutureInterval("", -INF, stm->operand[0].var, -delta);
            }
            else
            {
                interval[0][0] = new Interval(-INF, stm->operand[1].num);
                interval[0][1] = new Interval(stm->operand[1].num + delta, INF);
            }
            break;
        case GTR:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[1][0] = new FutureInterval("", -INF, stm->operand[0].var, -delta);
                interval[0][0] = new FutureInterval(stm->operand[1].var, delta, "", INF);
                interval[1][1] = new FutureInterval(stm->operand[0].var, 0, "", INF);
                interval[0][1] = new FutureInterval("", -INF, stm->operand[1].var, 0);
            }
            else
            {
                interval[0][0] = new Interval(stm->operand[1].num + delta, INF);
                interval[0][1] = new Interval(-INF, stm->operand[1].num);
            }
            break;
        case GEQ:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[1][0] = new FutureInterval("", -INF, stm->operand[0].var, 0);
                interval[0][0] = new FutureInterval(stm->operand[1].var, 0, "", INF);
                interval[1][1] = new FutureInterval(stm->operand[0].var, delta, "", INF);
                interval[0][1] = new FutureInterval("", -INF, stm->operand[1].var, -delta);
            }
            else
            {
                interval[0][0] = new Interval(stm->operand[1].num, INF);
                interval[0][1] = new Interval(-INF, stm->operand[1].num - delta);
            }
            break;
        case EQU:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[0][0] = new FutureInterval(stm->operand[1].var, 0, stm->operand[1].var, 0);
                interval[1][0] = new FutureInterval(stm->operand[0].var, 0, stm->operand[0].var, 0);
            }
            else
            {
                interval[0][0] = new Interval(stm->operand[1].num, stm->operand[1].num);
            }
            break;
        case NEQ:
            if (stm->operand[1].type == Operand::VAR)
            {
                interval[0][1] = new FutureInterval(stm->operand[1].var, 0, stm->operand[1].var, 0);
                interval[1][1] = new FutureInterval(stm->operand[0].var, 0, stm->operand[0].var, 0);
            }
            else
            {
                interval[0][1] = new Interval(stm->operand[1].num, stm->operand[1].num);
            }
            break;
        default:
            break;
        }
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
            {
                if (interval[i][j] == NULL) 
                    continue;
                interval[i][j]->Print();
                string var = stm->operand[i].var;
                string newVar = var + (j == 0 ? "_t" : "_f");
                bool changed = false;
                for (auto stmt: Traverse(stm->next[j], var))
                    for (auto &op: stmt->operand)
                        if (op.type == Operand::VAR && op.var == var)
                        {
                            op.var = newVar;
                            changed = true;
                        }
                if (!changed) continue;
                symtab[newVar] = SymEntry(symtab[var].type);
                Statement *newStm = new Statement();
                newStm->type = ITS;
                newStm->operand.push_back(newVar);
                newStm->operand.push_back(var);
                newStm->interval = interval[i][j];
                newStm->next[0] = stm->next[j];
                stm->next[j] = newStm;
            }
    }
}

SsaGraph::~SsaGraph()
{
    cout << "~SsaGraph:" << endl;
    for (auto stm: Traverse())
    {
        stm->Print();
        delete stm;
    }
}