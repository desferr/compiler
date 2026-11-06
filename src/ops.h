#pragma once
#include <vector>

enum class OpsKind { Var, Arr, Const, Label, Op };

enum class OpCode {
    Add, Sub, Mul, Div,
    Neg, Lt, Gt, Le,
    Ge, Eq, Ne, Index,
    Assign, Read, Write, Jf,
    J, Alloc
};

struct OpsItem {
    OpsKind kind = OpsKind::Op;
    OpCode op = OpCode::Add;
    int ref = 0;
    bool isReal = false;
    long long intValue = 0;
    double realValue = 0;
    int line = 0, col = 0;
};

using Ops = std::vector<OpsItem>;

const char* opSymbol(OpCode op);