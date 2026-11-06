#pragma once
#include <string>
#include <vector>

enum NonTerm {
    NT_P = 100, NT_R, NT_M, NT_A,
    NT_Q, NT_E, NT_C, NT_D,
    NT_S, NT_U, NT_T, NT_V,
    NT_F, NT_G, NT_H, NT_Z,
    NT_LAST
};
constexpr int NT_COUNT = NT_LAST - NT_P;

enum Action {
    A_NONE = 0, A_OPERAND, A_CONST, A_ADD,
    A_SUB, A_MUL, A_DIV, A_NEG,
    A_LT, A_GT, A_LE, A_GE,
    A_EQ, A_NE, A_INDEX, A_ASSIGN,
    A_READ, A_WRITE, A_ALLOC, A_PROG1,
    A_PROG2, A_PROG3, A_PROG4, A_PROG5,
    A_PROG6, A_PROG7
};

struct Rule {
    int leftSide;
    std::vector<int> rightSide;
    std::vector<int> act;
};

bool isNonTerminal(int sym);
const std::vector<Rule>& rules();
int selectRule(int nt, int lexType);
std::vector<int> expectedTerminals(int nt);
std::string symName(int sym);
std::string lexDisplay(int lexType);
std::string ruleText(int number);
const char* actionName(int act);