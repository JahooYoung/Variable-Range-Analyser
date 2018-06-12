#include "Utility.h"
using namespace std;

#include <iostream>

Interval::Interval() {}

Interval::Interval(const Lattice_Z &_low, const Lattice_Z &_high)
    : low(_low), high(_high) {}

Interval* Interval::Copy()
{
    return new Interval(*this);
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

FutureInterval::~FutureInterval() {}

void FutureInterval::Print()
{
    cout << "[ft(" << varLow << ")+" << deltaLow << 
            ",ft(" << varHigh << ")+" << deltaHigh << "]" << endl;
}