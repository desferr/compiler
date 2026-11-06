#include "grammar.h"
#include "lexer.h"

const std::vector<Rule> RULES = {
    {NT_P, {L_ARRAY, NT_R, NT_P}, {A_NONE, A_NONE, A_NONE}},
    {NT_P, {L_BEGIN, NT_A, NT_Q, L_END}, {A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_R, {L_ID, L_LEFTBRACKET, L_INT, L_RIGHTBRACKET, NT_M}, {A_PROG6, A_NONE, A_CONST, A_ALLOC, A_NONE}},
    {NT_M, {L_COMMA, L_ID, L_LEFTBRACKET, L_INT, L_RIGHTBRACKET, NT_M}, {A_NONE, A_PROG6, A_NONE, A_CONST, A_ALLOC, A_NONE}},
    {NT_M, {L_SEMICOLON}, {A_NONE}},
    {NT_A, {L_ID, NT_H, L_ASSIGN, NT_S, NT_Z}, {A_OPERAND, A_NONE, A_NONE, A_NONE, A_ASSIGN}},
    {NT_A, {L_IF, NT_C, L_THEN, NT_A, NT_E, NT_Z}, {A_NONE, A_NONE, A_PROG1, A_NONE, A_NONE, A_PROG3}},
    {NT_A, {L_WHILE, NT_C, L_DO, NT_A, NT_Z}, {A_PROG4, A_NONE, A_PROG1, A_NONE, A_PROG5}},
    {NT_A, {L_BEGIN, NT_A, NT_Q, L_END}, {A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_A, {L_READ, L_LEFTPARENTHES, L_ID, NT_H, L_RIGHTPARENTHES}, {A_NONE, A_NONE, A_OPERAND, A_NONE, A_READ}},
    {NT_A, {L_WRITE, L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES}, {A_NONE, A_NONE, A_NONE, A_WRITE}},
    {NT_Q, {L_SEMICOLON, NT_A, NT_Q}, {A_NONE, A_NONE, A_NONE}},
    {NT_Q, {}, {}},
    {NT_E, {L_ELSE, NT_A}, {A_PROG2, A_NONE}},
    {NT_E, {}, {}},
    {NT_C, {L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES, NT_V, NT_U, NT_D}, {A_NONE, A_NONE, A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_C, {L_ID, NT_H, NT_V, NT_U, NT_D}, {A_OPERAND, A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_C, {L_INT, NT_V, NT_U, NT_D}, {A_CONST, A_NONE, A_NONE, A_NONE}},
    {NT_C, {L_REAL, NT_V, NT_U, NT_D}, {A_CONST, A_NONE, A_NONE, A_NONE}},
    {NT_C, {L_PLUS, NT_G, NT_V, NT_U, NT_D}, {A_NONE, A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_C, {L_MINUS, NT_G, NT_V, NT_U, NT_D}, {A_NONE, A_NONE, A_NEG, A_NONE, A_NONE}},
    {NT_D, {L_LESSERTHAN, NT_S, NT_Z}, {A_NONE, A_NONE, A_LT}},
    {NT_D, {L_GREATERTHAN, NT_S, NT_Z}, {A_NONE, A_NONE, A_GT}},
    {NT_D, {L_LESSEREQUAL, NT_S, NT_Z}, {A_NONE, A_NONE, A_LE}},
    {NT_D, {L_GREATEREQUAL, NT_S, NT_Z}, {A_NONE, A_NONE, A_GE}},
    {NT_D, {L_EQUAL, NT_S, NT_Z}, {A_NONE, A_NONE, A_EQ}},
    {NT_D, {L_NOTEQUAL, NT_S, NT_Z}, {A_NONE, A_NONE, A_NE}},
    {NT_S, {L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES, NT_V, NT_U}, {A_NONE, A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_S, {L_ID, NT_H, NT_V, NT_U}, {A_OPERAND, A_NONE, A_NONE, A_NONE}},
    {NT_S, {L_INT, NT_V, NT_U}, {A_CONST, A_NONE, A_NONE}},
    {NT_S, {L_REAL, NT_V, NT_U}, {A_CONST, A_NONE, A_NONE}},
    {NT_S, {L_PLUS, NT_G, NT_V, NT_U}, {A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_S, {L_MINUS, NT_G, NT_V, NT_U}, {A_NONE, A_NONE, A_NEG, A_NONE}},
    {NT_U, {L_PLUS, NT_T, NT_U}, {A_NONE, A_NONE, A_ADD}},
    {NT_U, {L_MINUS, NT_T, NT_U}, {A_NONE, A_NONE, A_SUB}},
    {NT_U, {}, {}},
    {NT_T, {L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES, NT_V}, {A_NONE, A_NONE, A_NONE, A_NONE}},
    {NT_T, {L_ID, NT_H, NT_V}, {A_OPERAND, A_NONE, A_NONE}},
    {NT_T, {L_INT, NT_V}, {A_CONST, A_NONE}},
    {NT_T, {L_REAL, NT_V}, {A_CONST, A_NONE}},
    {NT_T, {L_PLUS, NT_G, NT_V}, {A_NONE, A_NONE, A_NONE}},
    {NT_T, {L_MINUS, NT_G, NT_V}, {A_NONE, A_NONE, A_NEG}},
    {NT_V, {L_MULTIPLICATION, NT_F, NT_V}, {A_NONE, A_NONE, A_MUL}},
    {NT_V, {L_DIVISION, NT_F, NT_V}, {A_NONE, A_NONE, A_DIV}},
    {NT_V, {}, {}},
    {NT_F, {L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES}, {A_NONE, A_NONE, A_NONE}},
    {NT_F, {L_ID, NT_H}, {A_OPERAND, A_NONE}},
    {NT_F, {L_INT}, {A_CONST}},
    {NT_F, {L_REAL}, {A_CONST}},
    {NT_F, {L_PLUS, NT_G}, {A_NONE, A_NONE}},
    {NT_F, {L_MINUS, NT_G, NT_Z}, {A_NONE, A_NONE, A_NEG}},
    {NT_G, {L_LEFTPARENTHES, NT_S, L_RIGHTPARENTHES}, {A_NONE, A_NONE, A_NONE}},
    {NT_G, {L_ID, NT_H}, {A_OPERAND, A_NONE}},
    {NT_G, {L_INT}, {A_CONST}},
    {NT_G, {L_REAL}, {A_CONST}},
    {NT_H, {L_LEFTBRACKET, NT_S, L_RIGHTBRACKET}, {A_PROG7, A_NONE, A_INDEX}},
    {NT_H, {}, {}},
    {NT_Z, {}, {}},
};

struct Table {
    int cell[NT_COUNT][L_EOF + 1];
    Table() {
        for (auto& row : cell) for (int& c : row) c = 0;
        for (size_t i = 0; i < RULES.size(); i++) {
            if (!RULES[i].rightSide.empty()) {
                cell[RULES[i].leftSide - NT_P][RULES[i].rightSide[0]] = (int)i + 1;
            }
        }
        for (size_t i = 0; i < RULES.size(); i++) {
            if (RULES[i].rightSide.empty()) {
                for (int l = 1; l <= L_EOF; l++) {
                    int& c = cell[RULES[i].leftSide - NT_P][l];
                    if (c == 0) c = (int)i + 1;
                }
            }
        }
    }
};

const Table& table() { 
    static const Table t;  
    return t;
}

const char* TERM_TEXT[L_EOF + 1] = {
    "?", "ID", "INT", "REAL",
    ":=", "+", "-", "*",
    "/", "(", ")", "[",
    "]", ",", ";", "<",
    ">", "<=", ">=", "=",
    "<>", "if", "then", "else",
    "while", "do", "begin", "end",
    "read", "write", "array", "┴"
};

const char* NT_TEXT[NT_COUNT] = {
    "P", "R", "M", "A",
    "Q", "E", "C", "D",
    "S", "U", "T", "V",
    "F", "G", "H", "Z"
};

const char* ACT_TEXT[] = {
    "", "a", "k", "+",
    "-", "*", "/", "-'",
    "<", ">", "<=", ">=",
    "=", "<>", "i", ":=",
    "r", "w", "m1", "1",
    "2", "3", "4", "5",
    "6", "7"
};

bool isNonTerminal(int sym) {
    return sym >= NT_P && sym < NT_LAST;
}

const std::vector<Rule>& rules() { return RULES; }

int selectRule(int nt, int lexType) {
    if (lexType < 1 || lexType > L_EOF) return 0;
    return table().cell[nt - NT_P][lexType];
}

std::vector<int> expectedTerminals(int nt) {
    std::vector<int> res;
    for (int l = 1; l <= L_EOF; l++) {
        int rule = table().cell[nt - NT_P][l];
        if (rule && !RULES[rule - 1].rightSide.empty()) res.push_back(l);
    }
    return res;
}

std::string symName(int sym) {
    if (isNonTerminal(sym)) return NT_TEXT[sym - NT_P];
    return (sym >= 1 && sym <= L_EOF)?TERM_TEXT[sym]:TERM_TEXT[0];
}

std::string lexDisplay(int l) {
    switch (l) {
        case L_ID: return "идентификатор";
        case L_INT: return "целое число";
        case L_REAL: return "вещественное число";
        case L_EOF: return "конец файла";
    }
    return "'" + symName(l) + "'";
}

std::string ruleText(int number) {
    const Rule& rule = RULES[number - 1];
    std::string s = symName(rule.leftSide) + " →";
    if (rule.rightSide.empty()) return s + " λ";
    for (int x : rule.rightSide) s += " " + symName(x);
    return s;
}

const char* actionName(int act) { return ACT_TEXT[act]; }