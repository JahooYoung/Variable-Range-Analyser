#include "Utility.h"
#include "ConstraintGraph.h"
using namespace std;

#include <iostream>

Interval::Interval() : empty(0), undefined(true) {}

Interval::Interval(bool _empty) : empty(_empty), undefined(true) {}

Interval::Interval(const Lattice_Z &_low, const Lattice_Z &_high)
    : low(_low), high(_high), empty(0), undefined(true) {}

Interval* Interval::Copy()
{
    return new Interval(*this);
}

void Interval::ConnectToVariable(SymbolTable &symtab) {}

void Interval::ConvertToInterval() {}

Interval::~Interval() {}

void Interval::Print()
{
    cout << '[' << low << ',' << high << ']' << endl;
}

/**
 * FutureInterval Block
 */

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

void FutureInterval::ConvertToInterval()
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

Interval calc(const Interval &A, enum StType type) {
    switch(type) {
        case I2F:
            return A;
        case F2I:
            return Interval(A.low.myceil(), A.high.myfloor());
        default:
            cerr << "error in calc func" << endl;
    }
    return Interval();
}

Interval calc(const Interval &A, const Interval &B, enum StType type) {
    if ((A.empty || B.empty) && type != PHI)
        return Interval(1);
    
    Lattice_Z a, b, c, d;
    switch(type) {
        case ADD:
            return Interval(A.low + B.low, A.high + B.high);

        case SUB:
            return Interval(A.low - B.high, A.high - B.low);

        case MUL:
            a = A.low * B.low;
            b = A.high * B.low;
            c = A.low * B.high;
            d = A.high * B.high;
            return Interval(min(min(a, b), min(c, d)),
                            max(max(a, b), max(c, d)));

        case DIV:
            if (B.low < 0 && 0 < B.high) 
                return Interval(-INF, INF);
            return calc(A, Interval(1 / B.high, 1 / B.low), MUL);

        case PHI:
            if (A.empty) return B;
            if (B.empty) return A;
            return Interval(min(A.low, B.low), max(A.high, B.high));
        
        case ITS:
            if (A.low < B.low && B.low < A.high) 
                return Interval(B.low, min(A.high, B.high));
            else if (B.low < A.low && A.low < B.high)
                return Interval(A.low, min(A.high, B.high));
            return Interval(1);
        
        default:
            cerr << "error in calc func" << endl;
            return Interval();
    }
}