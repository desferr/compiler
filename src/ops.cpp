#include "ops.h"

const char* opSymbol(OpCode op) {
    switch (op) {
        case OpCode::Add: return "+";
        case OpCode::Sub: return "-";
        case OpCode::Mul: return "*";
        case OpCode::Div: return "/";
        case OpCode::Neg: return "-'";
        case OpCode::Lt: return "<";
        case OpCode::Gt: return ">";
        case OpCode::Le: return "<=";
        case OpCode::Ge: return ">=";
        case OpCode::Eq: return "=";
        case OpCode::Ne: return "<>";
        case OpCode::Index: return "i";
        case OpCode::Assign: return ":=";
        case OpCode::Read: return "r";
        case OpCode::Write: return "w";
        case OpCode::Jf: return "jf";
        case OpCode::J: return "j";
        case OpCode::Alloc: return "m1";
    }
    return "?";
}