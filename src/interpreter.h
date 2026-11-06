#pragma once
#include "ops.h"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

struct RuntimeError : std::runtime_error {
    int line, col;
    RuntimeError(const std::string& msg, int l, int c): std::runtime_error(msg), line(l), col(c) {}
};

struct Value {
    bool isReal = false;
    long long i = 0;
    double d = 0;
    static Value Int(long long x) { 
        Value v;
        v.i = x;
        return v;
    }
    static Value Real(double x) {
        Value v;
        v.isReal = true;
        v.d = x;
        return v;
    }
};

std::string formatValue(const Value& v);

class Interpreter {
private:
    enum class Kind { Value, Bool, Label, VarRef, ArrRef, ElemRef };
    struct StackItem {
        Kind kind = Kind::Value;
        Value val;
        bool flag = false;
        int a = 0, b = 0;
        int line = 0, col = 0;
    };
    struct Slot {
        bool init = false;
        Value v;
    };
    struct Array {
        bool allocated = false;
        std::vector<Slot> data;
    };
    const Ops& ops_;
    const std::vector<std::string>& varNames_;
    const std::vector<std::string>& arrNames_;
    std::istream& in_;
    std::ostream& out_;
    std::ostream* trace_;
    std::vector<StackItem> stack_;
    std::vector<Slot> vars_;
    std::vector<Array> arrays_;

    StackItem pop(const OpsItem& it);
    void push(const StackItem& s) { stack_.push_back(s); }
    Value valueOf(const StackItem& s);
    void store(const StackItem& target, const Value& v, const OpsItem& it);
    Value arith(OpCode op, const Value& a, const Value& b, const OpsItem& it) const;
    static bool compare(OpCode op, const Value& a, const Value& b);
    void execute(const OpsItem& it, size_t& pc);
    void executeOp(const OpsItem& it, size_t& pc);
    void jumpTo(const StackItem& label, const OpsItem& it, size_t& pc) const;
    std::string elementText(const OpsItem& it) const;
    std::string describe(const StackItem& s) const;
    void traceStep(long long step, size_t pc, const OpsItem& it) const;
public:
    Interpreter(const Ops& ops, const std::vector<std::string>& varNames, const std::vector<std::string>& arrNames, std::istream& in, std::ostream& out, std::ostream* trace = nullptr);
    void run();
    void dumpMemory(std::ostream& os) const;
};