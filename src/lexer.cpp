#include "lexer.h"
#include <map>
#include <cstdlib>

enum Classes { 
    C_LETTER, C_DIGIT, C_DOT, C_SPACE,
    C_COLON, C_EQUAL, C_LESSERTHAN, C_GREATERTHAN,
    C_LEFTBRACES, C_RIGHTBRACES, C_SINGLE, C_OTHER,
    C_EOF, NCLASSES
};

enum States {
    ST_S, ST_I, ST_C, ST_D,
    ST_E, ST_A, ST_L, ST_M,
    ST_K, NSTATES
};

enum SpecSymbols { 
    Z = 100, Z_STAR = 101, ERROR = 102
};

enum SemProgs {
    NO = 0,
    P_START = 1,
    P_ADD = 2
};

struct Cell { int next; int act; };

static const Cell T[NSTATES][NCLASSES] = {
    {{ST_I, P_START}, {ST_C, P_START}, {ERROR, NO}, {ST_S, NO}, {ST_A, NO}, {Z, NO}, {ST_L, NO}, {ST_M, NO}, {ST_K, NO}, {ERROR, NO}, {Z, NO}, {ERROR, NO}, {Z, NO}},
    {{ST_I, P_ADD}, {ST_I, P_ADD}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}},
    {{ERROR, NO}, {ST_C, P_ADD}, {ST_D, P_ADD}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}},
    {{ERROR, NO}, {ST_E, P_ADD}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}},
    {{ERROR, NO}, {ST_E, P_ADD}, {ERROR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}},
    {{ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {Z, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}, {ERROR, NO}},
    {{Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z, NO}, {Z_STAR, NO}, {Z, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}},
    {{Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}, {Z_STAR, NO}},
    {{ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_K, NO}, {ST_S, NO}, {ST_K, NO}, {ST_K, NO}, {ERROR, NO}}
};

static int classOf(unsigned char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return C_LETTER;
    if (c >= '0' && c <= '9') return C_DIGIT;
    if (c == '.') return C_DOT;
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') return C_SPACE;
    if (c == ':') return C_COLON;
    if (c == '=') return C_EQUAL;
    if (c == '<') return C_LESSERTHAN;
    if (c == '>') return C_GREATERTHAN;
    if (c == '{') return C_LEFTBRACES;
    if (c == '}') return C_RIGHTBRACES;
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '[' || c == ']' || c == ',' || c == ';') return C_SINGLE;
    return C_OTHER;
}

static const std::map<std::string, int> KEYWORDS = {
    {"if", L_IF}, {"then", L_THEN}, {"else", L_ELSE}, {"while", L_WHILE},
    {"do", L_DO}, {"begin", L_BEGIN}, {"end", L_END}, {"read", L_READ},
    {"write", L_WRITE}, {"array", L_ARRAY}
};

static const std::map<char, int> SINGLE = {
    {'+', L_PLUS}, {'-', L_MINUS}, {'*', L_MULTIPLICATION}, {'/', L_DIVISION},
    {'(', L_LEFTPARENTHES}, {')', L_RIGHTPARENTHES}, {'[', L_LEFTBRACKET}, {']', L_RIGHTBRACKET},
    {',', L_COMMA}, {';', L_SEMICOLON}, {'=', L_EQUAL}
};

void Lexer::advance() {
    if (pos < src.size()) {
        if (src[pos] == '\n') { line++; col = 1; } else col++;
        pos++;
    }
}

static Lex finish(int st, char ch, int cls, const std::string& buffer, int line, int col) {
    Lex l;
    l.value = buffer;
    switch (st) {
        case ST_S:
            if (cls == C_EOF) { l.type = L_EOF; l.value = "EOF"; }
            else { l.type = SINGLE.at(ch); l.value = std::string(1, ch); }
            break;
        case ST_I:
            { 
                auto it = KEYWORDS.find(buffer); 
                l.type = (it != KEYWORDS.end())?it->second:L_ID; 
                break; 
            }
        case ST_C:
            l.type = L_INT;
            try { l.int_value = std::stoll(buffer); }
            catch (...) { throw LexError("Число слишком велико", line, col); }
            break;
        case ST_E:
            l.type = L_REAL;
            l.real_value = std::strtod(buffer.c_str(), nullptr);
            break;
        case ST_A:
            l.type = L_ASSIGN;
            l.value = ":=";
            break;
        case ST_L:
            l.type = (cls == C_EQUAL)?L_LESSEREQUAL:(cls == C_GREATERTHAN)?L_NOTEQUAL:L_LESSERTHAN;
            l.value = (cls == C_EQUAL)?"<=":(cls == C_GREATERTHAN)?"<>":"<";
            break;
        case ST_M:
            l.type = (cls == C_EQUAL)?L_GREATEREQUAL:L_GREATERTHAN;
            l.value = (cls == C_EQUAL)?">=":">";
            break;
    }
    return l;
}

static const char* errorText(int st) {
    if (st == ST_K) return "Незакрытый комментарий";
    if (st == ST_A) return "Ожидалось ':='";
    if (st == ST_D) return "Ожидалась цифра после десятичной точки";
    if (st == ST_C || st == ST_E) return "Недопустимый символ в числе";
    return "Недопустимый символ";
}

Lex Lexer::next() {
    int st = ST_S;
    std::string buffer;
    int sl = line, sc =col;
    for (;;) {
        if (st == ST_S) { sl = line; sc = col; }
        bool eof = (pos >= src.size());
        char ch = (eof)?'\0':src[pos];
        int cls = (eof)?C_EOF:classOf((unsigned char)ch);
        Cell c = T[st][cls];
        if (c.next == ERROR) throw LexError(errorText(st), (st == ST_K)?sl:line, (st == ST_K)?sc:col);
        if (c.next == Z || c.next == Z_STAR) {
            Lex l = finish(st, ch, cls, buffer, sl, sc);
            l.line = sl;
            l.col = sc;
            if (c.next == Z && !eof) advance();
            return l;
        }
        if (c.act == P_START) buffer = std::string(1, ch);
        if (c.act == P_ADD) buffer += ch;
        advance();
        st = c.next;
    }
}