#include "Utility.h"
using namespace std;

#include <iostream>

Interval::Interval() 
    : undefined(true) {}

Interval::Interval(const Lattice_Z &_low, const Lattice_Z &_high)
    : low(_low), high(_high), undefined(false) {}

Interval* Interval::Copy()
{
    return new Interval(*this);
}

void Interval::ConnectToVariable(SymbolTable &symtab)
{
    return;
}

Interval Interval::ConvertToInterval()
{
    return *this;
}

Interval::~Interval() {}

void Interval::Print()
{
    cout << '[' << low << ',' << high << ']' << endl;
}

FutureInterval::FutureInterval() {}

FutureInterval::FutureInterval(const string &_varLow, const Lattice_Z &_deltaLow,
                               const string &_varHigh, const Lattice_Z &_deltaHigh)
    : varLow(_varLow), varHigh(_varHigh), deltaLow(_deltaLow), deltaHigh(_deltaHigh) {}

FutureInterval* FutureInterval::Copy()
{
    return new FutureInterval(*this);
}

void FutureInterval::ConnectToVariable(SymbolTable &symtab)
{ 
    nodeLow = varLow != "" ? symtab[varLow].node : NULL;
    nodeHigh = varHigh != "" ? symtab[varHigh].node : NULL;
}

Interval FutureInterval::ConvertToInterval()
{
    low = deltaLow;
    high = deltaHigh;
    if (nodeLow != NULL)
        low = low + nodeLow->I.low;
    if (nodeHigh != NULL)
        high = high + nodeLow->I.high;
    undefined = false;
}

FutureInterval::~FutureInterval() {}

void FutureInterval::Print()
{
    cout << "[ft(" << varLow << ")+" << deltaLow << 
            ",ft(" << varHigh << ")+" << deltaHigh << "]" << endl;
}