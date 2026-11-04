#pragma once
#include <string>
#include <stdexcept>

enum LexType { 
    L_ID = 1, L_INT, L_REAL, L_ASSIGN,
    L_PLUS, L_MINUS, L_MULTIPLICATION, L_DIVISION,
    L_LEFTPARENTHES, L_RIGHTPARENTHES, L_LEFTBRACKET, L_RIGHTBRACKET,
    L_COMMA, L_SEMICOLON, L_LESSERTHAN, L_GREATERTHAN,
    L_LESSEREQUAL, L_GREATEREQUAL, L_EQUAL, L_NOTEQUAL,
    L_IF, L_THEN, L_ELSE, L_WHILE,
    L_DO, L_BEGIN, L_END, L_READ,
    L_WRITE, L_ARRAY, L_EOF
};

struct Lex {
    int type = 0;
    std::string value;
    long long int_value = 0;
    double real_value = 0;
    int line = 0;
    int col = 0;
};

struct LexError : std::runtime_error {
    int line, col;
    LexError(const std::string& msg, int l, int c) : std::runtime_error(msg), line(l), col(c) {}
};

class Lexer {
private:
    std::string src;
    size_t pos = 0;
    int line = 1, col = 1;
    void advance();
public:
    explicit Lexer(std::string text) : src(std::move(text)) {}
    Lex next();
};