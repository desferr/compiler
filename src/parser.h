#pragma once
#include "lexer.h"
#include "ops.h"
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct ParseError : std::runtime_error {
    int line, col;
    bool semantic;
    ParseError(const std::string& msg, int l, int c, bool sem): std::runtime_error(msg), line(l), col(c), semantic(sem) {}
};

class Parser {
private:
    struct StackItem {
        int sym;
        int act;
        int line, col;
    };
    Lexer lexer_;
    std::ostream* trace_;
    Lex cur_;
    std::vector<StackItem> stack_;
    std::vector<int> labels_;
    Ops ops_;
    std::vector<std::string> varNames_, arrNames_;
    std::unordered_map<std::string, int> varIndex_, arrIndex_;

    void runAction(const StackItem& it);
    void pushOp(OpCode op, const StackItem& it);
    int variableNumber(const std::string& name);
    void traceStep(int step, const StackItem& top, int rule);
    [[noreturn]] void syntaxErrorUnexpected(int nt) const;
    [[noreturn]] void syntaxErrorExpected(int terminal) const;
    std::string describeCurrent() const;
public:
    explicit Parser(Lexer lexer, std::ostream* trace = nullptr): lexer_(std::move(lexer)), trace_(trace) {}
    void parse();
    const Ops& ops() const { return ops_; }
    const std::vector<std::string>& variables() const { return varNames_; }
    const std::vector<std::string>& arrays() const { return arrNames_; }
    std::string itemText(const OpsItem& item) const;
};