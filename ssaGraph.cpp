#include "ssaGraph.h"
#include <vector>
#include <sstream>
#include <regex>
#include <iostream>
using namespace std;

SsaGraph::SsaGraph(string code)
{
    istringstream lines(code);
    regex symbols("[\\(\\)<>,;]");
    for (string line; getline(lines, line); )
    {
        line = regex_replace(line, symbols, " $0 ");
        istringstream is(line);
        string token;
        while (is >> token)
            cout << token << endl;
    }
}

SsaGraph::~SsaGraph()
{
    
}