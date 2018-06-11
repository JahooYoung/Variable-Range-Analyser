#ifndef Utility_H
#define Utility_H

#include "ConstraintGraph.h"
#include <iostream>
#include <string>
#include <map>
#include <cmath>

enum StType {I2F, F2I, ASN, ADD, SUB, MUL, DIV, PHI, ITS, CAL, LES, LEQ, GTR, GEQ, EQU, NEQ, NOP};

class SymEntry
{
public:
    enum Type {INT, FLOAT} type;
    Node* node;

    SymEntry() : type(INT), node(NULL) {}

    SymEntry(const Type &_type, Node *_node = NULL)
        : type(_type), node(_node) {}

    SymEntry(const std::string &_type, Node *_node = NULL)
        : node(_node)
    {
        if (_type == "int") 
            type = INT;
        else 
            type = FLOAT;
    }
};

typedef std::map<std::string, SymEntry> SymbolTable;

class Lattice_Z
{
private:
    double x;
public:
    Lattice_Z() : x(0) {}
    Lattice_Z(const double &y) : x(y) {}
    double operator = (const double &y)
    {
        return x = y;
    }
    bool operator < (const Lattice_Z &B)
    {
        return x < B.x;
    }
    double operator + (const Lattice_Z &B)
    {
        return x + B.x;
    }
    double operator - (const Lattice_Z &B)
    {
        return x - B.x;
    }
    double operator * (const Lattice_Z &B)
    {
        if (x == 0 || B.x == 0) return 0; // ?
        return x * B.x;
    }
    double operator / (const Lattice_Z &B)
    {
        if (B.x == 0) return 0; // ?
        return x / B.x;
    }
    friend std::ostream& operator << (std::ostream &out, const Lattice_Z &B)
    {
        return out << B.x;
    }
};

const double INF = INFINITY;

class Interval
{
private:
    Lattice_Z low, high;
    bool undefined;
public:
    Interval();
    Interval(const Lattice_Z &_low, const Lattice_Z &_high);
    virtual ~Interval();
    bool IsUndefined();
    virtual void Print();
};

class FutureInterval : public Interval
{
private:
    std::string varLow, varHigh;
    Lattice_Z deltaLow, deltaHigh;
public:
    FutureInterval();
    FutureInterval(const std::string &_varLow, const Lattice_Z &deltaLow,
                   const std::string &_varHigh, const Lattice_Z &deltaHigh);
    ~FutureInterval();
    void Print();
};



#endif