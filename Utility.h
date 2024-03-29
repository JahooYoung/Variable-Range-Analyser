#ifndef Utility_H
#define Utility_H

// #include "ConstraintGraph.h"
#include <iostream>
#include <string>
#include <map>
#include <cmath>

class Variable;

enum StType {I2F, F2I, ASN, ADD, SUB, ISUB, MUL, DIV, IDIV, PHI, ITS, CAL, LES, LEQ, GTR, GEQ, EQU, NEQ, NOP};

class SymEntry
{
public:
    enum Type {INT, FLOAT} type;
    Variable* node;

    SymEntry() : type(INT), node(NULL) {}

    SymEntry(const Type &_type, Variable *_node = NULL)
        : type(_type), node(_node) {}

    SymEntry(const std::string &_type, Variable *_node = NULL)
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
    double& operator = (const double &y)
    {
        return x = y;
    }
    friend bool operator < (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x < B.x;
    }
    friend bool operator <= (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x <= B.x;
    }
    friend bool operator > (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x > B.x;
    }
    friend bool operator >= (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x >= B.x;
    }
    friend bool operator == (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x == B.x;
    }
    friend Lattice_Z operator + (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x + B.x;
    }
    friend Lattice_Z operator - (const Lattice_Z &A, const Lattice_Z &B)
    {
        return A.x - B.x;
    }
    friend Lattice_Z operator * (const Lattice_Z &A, const Lattice_Z &B)
    {
        if (A.x == 0 || B.x == 0) return 0; // ?
        return A.x * B.x;
    }
    friend Lattice_Z operator / (const Lattice_Z &A, const Lattice_Z &B)
    {
        if (B.x == 0) return 0; // ?
        return A.x / B.x;
    }
    friend Lattice_Z min(const Lattice_Z &A, const Lattice_Z &B)
    {
        if (A < B) return A;
        return B;
    }
    friend Lattice_Z max(const Lattice_Z &A, const Lattice_Z &B)
    {
        if (A > B) return A;
        return B;
    }
    double myceil() const {
        return ceil(x - 1e-6);
    }
    double myfloor() const {
        return floor(x + 1e-6);
    }
    friend std::ostream& operator << (std::ostream &out, const Lattice_Z &B)
    {
        return out << B.x;
    }
};

const double INF = INFINITY;

class Interval
{
public:
    Lattice_Z low, high;
    bool empty;
    bool undefined;

    Interval();
    Interval(bool _empty);
    Interval(const Lattice_Z &_low, const Lattice_Z &_high);
    virtual Interval* Copy();
    virtual void ConnectToVariable(SymbolTable &symtab, Variable *node);
    virtual void ConvertToInterval();
    virtual ~Interval();
    virtual void Print() const;

    friend Interval calc(const Interval &A, enum StType type);
    friend Interval calc(const Interval &A, const Interval &B, enum StType type);
};

class FutureInterval : public Interval
{
private:
    std::string varLow, varHigh;
    Variable *nodeLow, *nodeHigh;
    Lattice_Z deltaLow, deltaHigh;
public:
    FutureInterval();
    FutureInterval(const std::string &_varLow, const Lattice_Z &deltaLow,
                   const std::string &_varHigh, const Lattice_Z &deltaHigh);
    FutureInterval* Copy();
    void ConnectToVariable(SymbolTable &symtab, Variable *node);
    void ConvertToInterval();
    ~FutureInterval();
    void Print() const;
};



#endif