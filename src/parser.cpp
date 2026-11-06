#include "parser.h"
#include "grammar.h"
#include <iomanip>
#include <sstream>

void Parser::parse() {
    ops_.clear();
    labels_.clear();
    stack_.clear();
    varNames_.clear();
    arrNames_.clear();
    varIndex_.clear();
    arrIndex_.clear();

    stack_.push_back({L_EOF, A_NONE, 0, 0});
    stack_.push_back({NT_P, A_NONE, 0, 0});
    cur_ = lexer_.next();

    if (trace_) {
        *trace_ << "=== Синтаксический анализ ===\n" << std::left << std::setw(5) << "No" << std::setw(16) << "Input" << std::setw(34) << "Rule / Match" << std::setw(8) << "Action" << "Stack\n";
    }

    int step = 0;
    for (;;) {
        StackItem top = stack_.back();
        step++;
        if (isNonTerminal(top.sym)) {
            int n = selectRule(top.sym, cur_.type);
            if (n == 0) syntaxErrorUnexpected(top.sym);
            traceStep(step, top, n);
            stack_.pop_back();
            runAction(top);
            const Rule& r = rules()[n - 1];
            for (size_t i = r.rightSide.size(); i-- > 0; ) {
                stack_.push_back({r.rightSide[i], r.act[i], cur_.line, cur_.col});
            }
        } else {
            if (top.sym != cur_.type) syntaxErrorExpected(top.sym);
            traceStep(step, top, 0);
            stack_.pop_back();
            runAction(top);
            if (top.sym == L_EOF) break;
            cur_ = lexer_.next();
        }
    }
}

int Parser::variableNumber(const std::string& name) {
    auto it = varIndex_.find(name);
    if (it != varIndex_.end()) return it->second;
    int n = (int)varNames_.size();
    varNames_.push_back(name);
    varIndex_[name] = n;
    return n;
}

void Parser::pushOp(OpCode op, const StackItem& it) {
    OpsItem o;
    o.kind = OpsKind::Op;
    o.op = op;
    o.line = it.line;
    o.col = it.col;
    ops_.push_back(o);
}

void Parser::runAction(const StackItem& it) {
    const int k = (int)ops_.size();
    switch (it.act) {
        case A_NONE: break;
        case A_OPERAND: {
            OpsItem o;
            o.line = cur_.line;
            o.col = cur_.col;
            auto a = arrIndex_.find(cur_.value);
            if (a != arrIndex_.end()) {
                o.kind = OpsKind::Arr;
                o.ref = a->second;
            } else {
                o.kind = OpsKind::Var;
                o.ref = variableNumber(cur_.value);
            }
            ops_.push_back(o);
            break;
        }
        case A_CONST: {
            OpsItem o;
            o.line = cur_.line;
            o.col = cur_.col;
            o.kind = OpsKind::Const;
            if (cur_.type == L_INT) {
                o.isReal = false;
                o.intValue = cur_.intValue;
            } else {
                o.isReal = true;
                o.realValue = cur_.realValue;
            }
            ops_.push_back(o);
            break;
        }
        case A_ADD: 
            pushOp(OpCode::Add, it);
            break;
        case A_SUB:
            pushOp(OpCode::Sub, it);
            break;
        case A_MUL:
            pushOp(OpCode::Mul, it);
            break;
        case A_DIV:
            pushOp(OpCode::Div, it);
            break;
        case A_NEG:
            pushOp(OpCode::Neg, it);
            break;
        case A_LT:
            pushOp(OpCode::Lt, it);
            break;
        case A_GT:
            pushOp(OpCode::Gt, it);
            break;
        case A_LE:
            pushOp(OpCode::Le, it);
            break;
        case A_GE:
            pushOp(OpCode::Ge, it);
            break;
        case A_EQ:
            pushOp(OpCode::Eq, it);
            break;
        case A_NE:
            pushOp(OpCode::Ne, it);
            break;
        case A_INDEX:
            pushOp(OpCode::Index, it);
            break;
        case A_ASSIGN:
            pushOp(OpCode::Assign, it);
            break;
        case A_READ:
            pushOp(OpCode::Read, it);
            break;
        case A_WRITE:
            pushOp(OpCode::Write, it);
            break;
        case A_ALLOC:
            pushOp(OpCode::Alloc, it);
            break;
        case A_PROG1: {
            labels_.push_back(k);
            OpsItem m;
            m.kind = OpsKind::Label;
            m.ref = -1;
            m.line = it.line;
            m.col = it.col;
            ops_.push_back(m);
            pushOp(OpCode::Jf, it);
            break;
        }
        case A_PROG2: {
            ops_[labels_.back()].ref = k + 2;
            labels_.pop_back();
            labels_.push_back(k);
            OpsItem m;
            m.kind = OpsKind::Label;
            m.ref = -1;
            m.line = it.line;
            m.col = it.col;
            ops_.push_back(m);
            pushOp(OpCode::J, it);
            break;
        }
        case A_PROG3: 
            ops_[labels_.back()].ref = k;
            labels_.pop_back();
            break;
        case A_PROG4:
            labels_.push_back(k);
            break;
        case A_PROG5: {
            ops_[labels_.back()].ref = k + 2;
            labels_.pop_back();
            OpsItem m;
            m.kind = OpsKind::Label;
            m.ref = labels_.back();
            m.line = it.line;
            m.col = it.col;
            labels_.pop_back();
            ops_.push_back(m);
            pushOp(OpCode::J, it);
            break;
        }
        case A_PROG6: {
            if (arrIndex_.count(cur_.value)) {
                throw ParseError("повторное описание массива '" + cur_.value + "'", cur_.line, cur_.col, true);
            }
            int n = (int)arrNames_.size();
            arrNames_.push_back(cur_.value);
            arrIndex_[cur_.value] = n;
            OpsItem o;
            o.kind = OpsKind::Arr;
            o.ref = n;
            o.line = cur_.line;
            o.col = cur_.col;
            ops_.push_back(o);
            break;
        }
        case A_PROG7:
            if (ops_.empty() || ops_.back().kind != OpsKind::Arr) {
                std::string name = (!ops_.empty() && ops_.back().kind == OpsKind::Var)?"'" + varNames_[ops_.back().ref] + "' ":"";
                throw ParseError(name + "не является массивом, индексация невозможна", cur_.line, cur_.col, true);
            }
            break;
    }
}

std::string Parser::describeCurrent() const {
    switch(cur_.type) {
        case L_ID: return "идентификатор '" + cur_.value + "'";
        case L_INT: return "число " + cur_.value;
        case L_REAL: return "число " + cur_.value;
        case L_EOF: return "конец файла";
    }
    return lexDisplay(cur_.type);
}

void Parser::syntaxErrorUnexpected(int nt) const {
    std::string msg = "неожиданная лексема: " + describeCurrent() + "; ожидалось: ";
    std::vector<int> exp = expectedTerminals(nt);
    for (size_t i = 0; i < exp.size(); i++) {
        if (i) msg += ", ";
        msg += lexDisplay(exp[i]);
    }
    throw ParseError(msg, cur_.line, cur_.col, false);
}

void Parser::syntaxErrorExpected(int terminal) const {
    std::string msg;
    if (terminal == L_EOF) msg = "лишний текст после конца программы: " + describeCurrent();
    else msg = "ожидалось " + lexDisplay(terminal) + ", получено " + describeCurrent();
    throw ParseError(msg, cur_.line, cur_.col, false);
}

static std::string pad(const std::string& s, size_t width) {
    size_t chars = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) chars++;
    return (chars >= width)?s:s + std::string(width - chars, ' ');
}

void Parser::traceStep(int step, const StackItem& top, int rule) {
    if (!trace_) return;
    std::string in = symName(cur_.type);
    if (cur_.type == L_ID || cur_.type == L_INT || cur_.type == L_REAL) in += "(" + cur_.value + ")";
    std::string what = rule?"#" + std::to_string(rule) + " " + ruleText(rule):"match " + symName(top.sym);
    std::string act = top.act?actionName(top.act):"";
    *trace_ << std::left << std::setw(5) << step << pad(in, 16) << pad(what, 34) << pad(act, 8);
    for (size_t i = stack_.size(); i-- > 0; ) *trace_ << symName(stack_[i].sym) << ' ';
    *trace_ << '\n';
}

std::string Parser::itemText(const OpsItem& it) const {
    switch (it.kind) {
        case OpsKind::Var: return varNames_[it.ref];
        case OpsKind::Arr: return arrNames_[it.ref];
        case OpsKind::Const: {
            std::ostringstream s;
            if (it.isReal) s << it.realValue;
            else s << it.intValue;
            return s.str();
        }
        case OpsKind::Label: return (it.ref < 0)?"m?":"m" + std::to_string(it.ref);
        case OpsKind::Op: return opSymbol(it.op);
    }
    return "?";
}