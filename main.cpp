#include "VariableRangeAnalyser.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <regex>
#include <iostream>
using namespace std;

void PrintHelp()
{
    printf("Wrong number of parameters!\n");
    printf("Usage: ./vra ./t1.ssa\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        PrintHelp();
        return 0;
    }

    ifstream ssaFile(argv[1]);
    string line, funcSection, funcName;
    regex funcDec(";; Function (\\w+) .*");
    while (getline(ssaFile, line))
    {
        if (line == "")
            continue;
        smatch sm;
        if (regex_match(line, sm, funcDec))
        {
            if (funcSection != "")
                func[funcName] = new VariableRangeAnalyser(funcSection);
            funcSection = "";
            funcName = sm[1];
        }
        else
            funcSection += line + '\n';
    }
    if (funcSection != "")
        func[funcName] = new VariableRangeAnalyser(funcSection);

    VariableRangeAnalyser *foo = func["foo"];
    foo->ExecuteWithStdio();

    for (auto vra: func)
        delete vra.second;
    return 0;
}