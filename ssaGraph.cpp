#include "SsaGraph.h"
#include <vector>
#include <sstream>
#include <regex>
#include <iostream>
using namespace std;

// Operand::Operand() {}

Statement::Statement() : visited(false) 
{
    next[0] = next[1] = NULL;
}

void Operand::Print()
{
    switch (type)
    {
    case VAR:
        cout << var << ' ';
        break;
    case INT:
        cout << intNum << ' ';
        break;
    case FLOAT:
        cout << floatNum << ' ';
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
public:
    TokenStream(string s)
    {
        istringstream iss(s);
        while (iss >> s) 
            tokens.push_back(s);
        pt = tokens.begin();
    }
    void Append(string s)
    {
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

Operand FetchOperand(TokenStream &tokens)
{
    Operand operand;
    if ('0' <= tokens.Now()[0] && tokens.Now()[0] <= '9')
    {
        if (tokens.Now().find('.') == string::npos)
        {
            operand.type = Operand::INT;
            operand.intNum = stoi(tokens.Now());
        }
        else
        {
            operand.type = Operand::FLOAT;
            operand.floatNum = stod(tokens.Now());
        }
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
            stm->type = tokens.Now() == "<" ? Statement::LEQ : Statement::GEQ;
        else
            stm->type = tokens.Now() == "<" ? Statement::LES : Statement::GTR;
    }
    else if (tokens.Now() == "!")
        stm->type = Statement::NEQ;
    else 
        stm->type = Statement::EQU;
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
            string type = symtab[regex_replace(var, regex("^(\\w+)_\\d+$"), "$1")].first;
            symtab[var] = SymEntry(type, NULL);
        }
        for (tokens.Forward(2); tokens.Now() != ">"; )
        {
            tokens.Forward();
            stm->operand.push_back(FetchOperand(tokens));
        }
        stm->type = Statement::PHI;
        return stm;
    }
    else
    {
        stm->operand.push_back(FetchOperand(tokens));
        string var = stm->operand[0].var;
        if (!symtab.count(var)) 
        {
            string type = symtab[regex_replace(var, regex("^(\\w+)_\\d+$"), "$1")].first;
            symtab[var] = SymEntry(type, NULL);
        }
        tokens.Forward();
        if (tokens.Now() == "(")
        {
            tokens.Forward();
            if (tokens.Now() == "int")
                stm->type = Statement::F2I;
            else
                stm->type = Statement::I2F;
            tokens.Forward(2);
            stm->operand.push_back(FetchOperand(tokens));
            return stm;
        }
        stm->operand.push_back(FetchOperand(tokens));
            // cout << "aaa:" << tokens.Now() << endl;
        if (tokens.Now() == ";")
        {
            stm->type = Statement::ASN;
            return stm;
        }
        if (tokens.Now() == "(")
        {
            stm->type = Statement::CAL;
            while (tokens.Now() != ")")
            {
                tokens.Forward();
                cout << tokens.Now() << endl;
                stm->operand.push_back(FetchOperand(tokens));
            }
            return stm;
        }
        if (tokens.Now() == "+")
            stm->type = Statement::ADD;
        else if (tokens.Now() == "-")
            stm->type = Statement::SUB;
        else if (tokens.Now() == "*")
            stm->type = Statement::MUL;
        else if (tokens.Now() == "/")
            stm->type = Statement::DIV;
        tokens.Forward();
        stm->operand.push_back(FetchOperand(tokens));
        return stm;
    }
}

SsaGraph::SsaGraph(string code, SymbolTable &symtab, vector<string> &parameters)
{
    istringstream lines(code);
    regex symbols("[\\(\\)<>,;]");
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
        line = regex_replace(line, symbols, " $0 ");
        // cout << line << endl;
        TokenStream tokens(line);
        Statement *stm;
        if (tokens.Now() == "if") 
        {
            for (int i = 0; i < 3; i++)
            {
                getline(lines, line);
                line = regex_replace(line, symbols, " $0 ");
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
                stm->type = Statement::NOP;
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
            symtab[tokens.Peek(1)] = SymEntry(tokens.Now(), NULL);
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
                symtab[tokens.Peek(1)] = SymEntry(tokens.Now(), NULL);
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

SsaGraph::~SsaGraph()
{
    cout << "~SsaGraph" << endl;
    vector<Statement*> que;
    que.push_back(entry);
    entry->visited = true;
    for (size_t i = 0; i < que.size(); i++)
    {
        Statement *u = que[i];
        u->Print();
        for (int j = 0; j < 2; j++)
            if (u->next[j] != NULL && !u->next[j]->visited)
            {
                que.push_back(u->next[j]);
                u->next[j]->visited = true;
            }
    }
    for (Statement *i: que)
        delete i;
}