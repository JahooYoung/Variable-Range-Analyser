#include "Utility.h"
#include "ConstraintGraph.h"
using namespace std;

#include <iostream>

Interval::Interval() : empty(false), undefined(true) {}

Interval::Interval(bool _empty) : empty(_empty), undefined(false) {}

Interval::Interval(const Lattice_Z &_low, const Lattice_Z &_high)
    : low(_low), high(_high), empty(false), undefined(false) {}

Interval* Interval::Copy()
{
    return new Interval(*this);
}

void Interval::ConnectToVariable(SymbolTable &symtab, Variable *node) {}

void Interval::ConvertToInterval() {}

Interval::~Interval() {}

void Interval::Print() const
{
    if (undefined)
        cout << "undefined" << endl;
    else if (empty)
        cout << "empty" << endl;
    else
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
    // nodeLow and nodeHigh !!!
    FutureInterval* ft = new FutureInterval();
    ft->nodeLow = nodeLow != NULL ? nodeLow->newNode : NULL;
    ft->nodeHigh = nodeHigh != NULL ? nodeHigh->newNode : NULL;
    ft->deltaLow = deltaLow;
    ft->deltaHigh = deltaHigh;
    return ft;
}

void FutureInterval::ConnectToVariable(SymbolTable &symtab, Variable *node)
{ 
    if (varLow != "")
    {
        nodeLow = symtab[varLow].node;
        nodeLow->outEdge.push_back(node);
    }
    else 
        nodeLow = NULL;
    if (varHigh != "")
    {
        nodeHigh = symtab[varHigh].node;
        if (varHigh != varLow) nodeHigh->outEdge.push_back(node);
    }
    else
        nodeHigh = NULL;
}

void FutureInterval::ConvertToInterval()
{
    low = deltaLow;
    high = deltaHigh;
    if (nodeLow != NULL)
        low = low + nodeLow->I.low;
    if (nodeHigh != NULL)
        high = high + nodeHigh->I.high;
    undefined = false;
}

FutureInterval::~FutureInterval() {}

void FutureInterval::Print() const
{
    cout << "[ft(" << varLow << ")+" << deltaLow << 
            ",ft(" << varHigh << ")+" << deltaHigh << "]" << endl;
}

/**
 * Interval Operation Block
 */

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

Interval calc(const Interval &A, const Interval &B, enum StType type) 
{
    if ((A.empty || B.empty) && type != PHI)
        return Interval(1);
    
    Lattice_Z a, b, c, d;
    switch(type) 
    {
        case ADD:
            if (A.undefined || B.undefined) 
                return Interval();
            return Interval(A.low + B.low, A.high + B.high);

        case SUB:
            // A.Print();
            // B.Print();
            if (A.undefined || B.undefined) 
                return Interval();
            return Interval(A.low - B.high, A.high - B.low);

        case MUL:
            if (A.undefined || B.undefined) 
                return Interval();
            a = A.low * B.low;
            b = A.high * B.low;
            c = A.low * B.high;
            d = A.high * B.high;
            return Interval(min(min(a, b), min(c, d)),
                            max(max(a, b), max(c, d)));

        case DIV:
            if (A.undefined || B.undefined) 
                return Interval();
            if (B.low < 0 && 0 < B.high) 
                return Interval(-INF, INF);
            return calc(A, Interval(1 / B.high, 1 / B.low), MUL);

        case PHI:
            if (A.undefined || A.empty) return B;
            if (B.undefined || B.empty) return A;
            return Interval(min(A.low, B.low), max(A.high, B.high));
        
        case ITS:
            if (A.undefined || A.empty) return B;
            if (B.undefined || B.empty) return A;
            if (A.low <= B.low && B.low <= A.high) 
                return Interval(B.low, min(A.high, B.high));
            else if (B.low <= A.low && A.low <= B.high)
                return Interval(A.low, min(A.high, B.high));
            return Interval(1);
        
        default:
            cerr << "error in calc func" << endl;
            return Interval();
    }
}