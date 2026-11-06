#include "interpreter.h"
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

const long long MAX_ARRAY_SIZE = 100000000;

double toReal(const Value& v) { return (v.isReal)?v.d:(double)v.i; }

bool parseNumber(const std::string& tok, Value& res) {
    size_t p = 0;
    if (p < tok.size() && (tok[p] == '+' || tok[p] == '-')) p++;
    size_t d0 = p;
    while (p < tok.size() && tok[p] >= '0' && tok[p] <= '9') p++;
    if (p == d0) return false;
    if (p == tok.size()) {
        errno = 0;
        char* end = nullptr;
        long long x = std::strtoll(tok.c_str(), &end, 10);
        if (errno == ERANGE) return false;
        res = Value::Int(x);
        return true;
    }
    if (tok[p] != '.') return false;
    p++;
    size_t f0 = p;
    while (p < tok.size() && tok[p] >= '0' && tok[p] <= '9') p++;
    if (p == f0 || p != tok.size()) return false;
    res = Value::Real(std::strtod(tok.c_str(), nullptr));
    return true;
}

std::string formatValue(const Value& v) {
    if (!v.isReal) return std::to_string(v.i);
    char buffer[64];
    std::snprintf(buffer, sizeof buffer, "%.10g", v.d);
    std::string s = buffer;
    if (s.find_first_of(".e") == std::string::npos) s += ".0";
    return s;
}

Interpreter::Interpreter(const Ops& ops, const std::vector<std::string>& varNames, const std::vector<std::string>& arrNames, std::istream& in, std::ostream& out, std::ostream* trace): ops_(ops), varNames_(varNames), arrNames_(arrNames), in_(in), out_(out), trace_(trace), vars_(varNames.size()), arrays_(arrNames.size()) {}

void Interpreter::run() {
    stack_.clear();
    vars_.assign(varNames_.size(), Slot());
    arrays_.assign(arrNames_.size(), Array());
    if (trace_) {
        *trace_ << "=== Выполнение ОПС ===\n" << std::left << std::setw(7) << "Step" << std::setw(6) << "No" << std::setw(10) << "Element" << "Stack\n";
    }
    size_t pc = 0;
    long long step = 0;
    while (pc < ops_.size()) {
        const OpsItem& it = ops_[pc];
        size_t current = pc;
        pc++;
        execute(it, pc);
        traceStep(++step, current, it);
    }
}

void Interpreter::execute(const OpsItem& it, size_t& pc) {
    StackItem s;
    s.line = it.line;
    s.col = it.col;
    switch (it.kind) {
        case OpsKind::Var:
            s.kind = Kind::VarRef;
            s.a = it.ref;
            push(s);
            break;
        case OpsKind::Arr:
            s.kind = Kind::ArrRef;
            s.a = it.ref;
            push(s);
            break;
        case OpsKind::Const:
            s.kind = Kind::Value;
            s.val = (it.isReal)?Value::Real(it.realValue):Value::Int(it.intValue);
            push(s);
            break;
        case OpsKind::Label:
            s.kind = Kind::Label;
            s.a = it.ref;
            push(s);
            break;
        case OpsKind::Op:
            executeOp(it, pc);
            break;
    }
}

Interpreter::StackItem Interpreter::pop(const OpsItem& it) {
    if (stack_.empty()) {
        throw RuntimeError("внутренняя ошибка: магазин операндов пуст", it.line, it.col);
    }
    StackItem s = stack_.back();
    stack_.pop_back();
    return s;
}

Value Interpreter::valueOf(const StackItem& s) {
    switch (s.kind) {
        case Kind::Value:
            return s.val;
        case Kind::VarRef: {
            const Slot& slot = vars_[s.a];
            if (!slot.init) {
                throw RuntimeError("переменная '" + varNames_[s.a] + "' не инициализирована", s.line, s.col);
            }
            return slot.v;
        }
        case Kind::ElemRef: {
            const Slot& slot = arrays_[s.a].data[s.b];
            if (!slot.init) {
                throw RuntimeError("элемент '" + arrNames_[s.a] + "[" + std::to_string(s.b) + "]' не инициализирован", s.line, s.col);
            }
            return slot.v;
        }
        case Kind::ArrRef: {
            throw RuntimeError("массив '" + arrNames_[s.a] + "' используется без индекса", s.line, s.col);
        }
        default: {
            throw RuntimeError("внутренняя ошибка: недопустимый операнд", s.line, s.col);
        }
    }
}

void Interpreter::store(const StackItem& target, const Value& v, const OpsItem& it) {
    Slot slot;
    slot.init = true;
    slot.v = v;
    if (target.kind == Kind::VarRef) vars_[target.a] = slot;
    else if (target.kind == Kind::ElemRef) arrays_[target.a].data[target.b] = slot;
    else throw RuntimeError("внутренняя ошибка: значение записывается не в переменную", it.line, it.col);
}

Value Interpreter::arith(OpCode op, const Value& a, const Value& b, const OpsItem& it) const {
    if (op == OpCode::Div) {
        double y = toReal(b);
        if (y == 0.0) throw RuntimeError("деление на ноль", it.line, it.col);
        double r = toReal(a) / y;
        if (!std::isfinite(r)) throw RuntimeError("переполнение вещественного числа", it.line, it.col);
        return Value::Real(r);
    }
    if (!a.isReal && !b.isReal) {
        long long r = 0;
        bool overflow = false;
        switch (op) {
            case OpCode::Add:
                overflow = __builtin_add_overflow(a.i, b.i, &r);
                break;
            case OpCode::Sub:
                overflow = __builtin_sub_overflow(a.i, b.i, &r);
                break;
            default:
                overflow = __builtin_mul_overflow(a.i, b.i, &r);
                break;
        }
        if (overflow) throw RuntimeError("переполнение целого числа", it.line, it.col);
        return Value::Int(r);
    }
    double x = toReal(a), y = toReal(b), r = 0;
    switch (op) {
        case OpCode::Add:
            r = x + y;
            break;
        case OpCode::Sub:
            r = x - y;
            break;
        default:
            r = x * y;
            break;
    }
    if (!std::isfinite(r)) throw RuntimeError("переполнение вещественного числа", it.line, it.col);
    return Value::Real(r);
}

bool Interpreter::compare(OpCode op, const Value& a, const Value& b) {
    if (!a.isReal && !b.isReal) {
        switch (op) {
            case OpCode::Lt: return a.i < b.i;
            case OpCode::Gt: return a.i > b.i;
            case OpCode::Le: return a.i <= b.i;
            case OpCode::Ge: return a.i >= b.i;
            case OpCode::Eq: return a.i == b.i;
            default: return a.i != b.i;
        }
    }
    double x = toReal(a), y = toReal(b);
    switch (op) {
        case OpCode::Lt: return x < y;
        case OpCode::Gt: return x > y;
        case OpCode::Le: return x <= y;
        case OpCode::Ge: return x >= y;
        case OpCode::Eq: return x == y;
        default: return x != y;
    }
}

void Interpreter::jumpTo(const StackItem& label, const OpsItem& it, size_t& pc) const {
    if (label.kind != Kind::Label || label.a < 0 || (size_t)label.a > ops_.size()) {
        throw RuntimeError("внутренняя ошибка: недопустимая метка перехода", it.line, it.col);
    }
    pc = (size_t)label.a;
}

void Interpreter::executeOp(const OpsItem& it, size_t& pc) {
    switch (it.op) {
        case OpCode::Add: case OpCode::Sub: case OpCode::Mul: case OpCode::Div: {
            StackItem sb = pop(it), sa = pop(it);
            Value a = valueOf(sa);
            Value b = valueOf(sb);
            StackItem r;
            r.val = arith(it.op, a, b, it);
            r.line = it.line;
            r.col = it.col;
            push(r);
            break;
        }
        case OpCode::Neg: {
            StackItem sa = pop(it);
            Value a = valueOf(sa);
            if (a.isReal) a.d = -a.d;
            else {
                if (a.i == (-9223372036854775807LL - 1)) {
                    throw RuntimeError("переполнение целого числа", it.line, it.col);
                }
                a.i = -a.i;
            }
            StackItem r;
            r.val = a;
            r.line = it.line;
            r.col = it.col;
            push(r);
            break;
        }
        case OpCode::Lt: case OpCode::Gt: case OpCode::Le: case OpCode::Ge: case OpCode::Eq: case OpCode::Ne: {
            StackItem sb = pop(it), sa = pop(it);
            Value a = valueOf(sa);
            Value b = valueOf(sb);
            StackItem r;
            r.kind = Kind::Bool;
            r.flag = compare(it.op, a, b);
            r.line = it.line;
            r.col = it.col;
            push(r);
            break;
        }
        case OpCode::Index: {
            StackItem sidx = pop(it), sarr = pop(it);
            Value idx = valueOf(sidx);
            if (sarr.kind != Kind::ArrRef) {
                throw RuntimeError("внутренняя ошибка: индексируется не массив", it.line, it.col);
            }
            if (idx.isReal) {
                throw RuntimeError("индекс массива должен быть целым числом", it.line, it.col);
            }
            const Array& arr = arrays_[sarr.a];
            if (!arr.allocated) {
                throw RuntimeError("память под массив '" + arrNames_[sarr.a] + "' не выделена", it.line, it.col);
            }
            if (idx.i < 0 || idx.i >= (long long)arr.data.size()) {
                throw RuntimeError("индекс " + std::to_string(idx.i) + " вне границ массива '" + arrNames_[sarr.a] + "' (размер " + std::to_string(arr.data.size()) + ")", it.line, it.col);
            }
            StackItem r;
            r.kind = Kind::ElemRef;
            r.a = sarr.a;
            r.b = (int)idx.i;
            r.line = it.line;
            r.col = it.col;
            push(r);
            break;
        }
        case OpCode::Assign: {
            StackItem sval = pop(it), starget = pop(it);
            store(starget, valueOf(sval), it);
            break;
        }
        case OpCode::Read: {
            StackItem starget = pop(it);
            std::string tok;
            if (!(in_ >> tok)) {
                throw RuntimeError("неожиданный конец ввода", it.line, it.col);
            }
            Value v;
            if (!parseNumber(tok, v)) {
                throw RuntimeError("во вводе ожидалось число, получено '" + tok + "'", it.line, it.col);
            }
            store(starget, v, it);
            break;
        }
        case OpCode::Write: {
            StackItem s = pop(it);
            out_ << formatValue(valueOf(s)) << "\n";
            break;
        }
        case OpCode::J: {
            StackItem label = pop(it);
            jumpTo(label, it, pc);
            break;
        }
        case OpCode::Jf: {
            StackItem label = pop(it), cond = pop(it);
            if (cond.kind != Kind::Bool) {
                throw RuntimeError("внутренняя ошибка: условие не является логическим значением", it.line, it.col);
            }
            if (!cond.flag) jumpTo(label, it, pc);
            break;
        }
        case OpCode::Alloc: {
            StackItem sn = pop(it), sarr = pop(it);
            Value n = valueOf(sn);
            if (sarr.kind != Kind::ArrRef) {
                throw RuntimeError("внутренняя ошибка: память выделяется не массиву", it.line, it.col);
            }
            if (n.isReal) {
                throw RuntimeError("размер массива должен быть целым числом", it.line, it.col);
            }
            if (n.i <= 0) {
                throw RuntimeError("размер массива должен быть положительным", it.line, it.col);
            }
            if (n.i > MAX_ARRAY_SIZE) {
                throw RuntimeError("размер массива слишком велик", it.line, it.col);
            }
            Array& arr = arrays_[sarr.a];
            if (arr.allocated) {
                throw RuntimeError("память под массив '" + arrNames_[sarr.a] + "' уже выделена", it.line, it.col);
            }
            arr.allocated = true;
            arr.data.assign((size_t)n.i, Slot());
            break;
        }
    }
}

std::string Interpreter::elementText(const OpsItem& it) const {
    switch (it.kind) {
        case OpsKind::Var: return varNames_[it.ref];
        case OpsKind::Arr: return arrNames_[it.ref];
        case OpsKind::Const: return (it.isReal)?formatValue(Value::Real(it.realValue)):std::to_string(it.intValue);
        case OpsKind::Label: return "m" + std::to_string(it.ref);
        case OpsKind::Op: return opSymbol(it.op);
    }
    return "?";
}

std::string Interpreter::describe(const StackItem& s) const {
    switch (s.kind) {
        case Kind::Value: return formatValue(s.val);
        case Kind::Bool: return (s.flag)?"true":"false";
        case Kind::Label: return "m" + std::to_string(s.a);
        case Kind::VarRef: return varNames_[s.a];
        case Kind::ArrRef: return arrNames_[s.a];
        case Kind::ElemRef: return arrNames_[s.a] + "[" + std::to_string(s.b) + "]";
    }
    return "?";
}

void Interpreter::traceStep(long long step, size_t pc, const OpsItem& it) const {
    if (!trace_) return;
    *trace_ << std::left << std::setw(7) << step << std::setw(6) << pc << std::setw(10) << elementText(it);
    for (const StackItem& s : stack_) *trace_ << describe(s) << ' ';
    *trace_ << '\n';
}

void Interpreter::dumpMemory(std::ostream& os) const {
    os << "=== Память ===\n";
    for (size_t i = 0; i < vars_.size(); i++) {
        os << varNames_[i] << " = " << ((vars_[i].init)?formatValue(vars_[i].v):std::string("<не определена>")) << "\n";
    }
    for (size_t i = 0; i < arrays_.size(); i++) {
        const Array& arr = arrays_[i];
        if (!arr.allocated) {
            os << arrNames_[i] << ": память не выделена\n";
            continue;
        }
        os << arrNames_[i] << "[" << arr.data.size() << "]:";
        size_t defined = 0;
        for (size_t k = 0; k < arr.data.size(); k++) {
            if (!arr.data[k].init) continue;
            defined++;
            os << " [" << k << "]=" << formatValue(arr.data[k].v);
        }
        if (defined == 0) os << " <ни один элемент не определён>";
        os << "\n";
    }
}