#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

struct Options {
    std::string inputFile;
    std::string logFile;
    bool showHelp = false;
    bool showLexs = false;
    bool lexsVerbose = false;
    bool showTrace = false;
    bool showOps = false;
    bool opsVerbose = false;
    bool showTables = false;
    bool showExec = false;
    bool showMemory = false;
    bool compileOnly = false;
};

static void printUsage(std::ostream& out) {
    out << "Использование: compiler.exe [флаги] <файл>\n"
        << "Флаги:\n"
        << "    -h, --help  показать эту справку\n"
        << "    -l <файл>   вывод логов работы в файл\n"
        << "    -t          вывод списка лексем\n"
        << "    -tv         вывод списка лексем с подробностями\n"
        << "    -p          логирование работы синтаксического анализатора\n"
        << "    -o          вывод ОПС\n"
        << "    -ov         вывод ОПС таблицей\n"
        << "    -s          вывод таблиц переменных и массивов\n"
        << "    -e          логирование выполнения ОПС\n"
        << "    -d          вывод состояния памяти после выполнения\n"
        << "    -c          только трансляция, без выполнения программы\n"
        << "Коды завершения программы:\n"
        << "    0 - успех\n"
        << "    1 - ошибка запуска\n"
        << "    2 - ошибка в программе\n";
}

static bool parseArgs(int argc, char* argv[], Options& opt) {
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "-h" || a == "--help") {
            opt.showHelp = true;
        } else if (a == "-l") {
            if (i + 1 >= argc) {
                std::cerr << "Ошибка: после -l нужно указать имя файла\n";
                return false;
            }
            opt.logFile = argv[++i];
        } else if (a == "-t") {
            opt.showLexs = true;
        } else if (a == "-tv") {
            opt.showLexs = true;
            opt.lexsVerbose = true;
        } else if (a == "-p") {
            opt.showTrace = true;
        } else if (a == "-o") {
            opt.showOps = true;
        } else if (a == "-ov") {
            opt.showOps = true;
            opt.opsVerbose = true;
        } else if (a == "-s") {
            opt.showTables = true;
        } else if (a == "-e") {
            opt.showExec = true;
        } else if (a == "-d") {
            opt.showMemory = true;
        } else if (a == "-c") {
            opt.compileOnly = true;
        } else if (!a.empty() && a[0] == '-') {
            std::cerr << "Ошибка: неизвестный флаг " << a << "\n";
            return false;
        } else {
            if (!opt.inputFile.empty()) {
                std::cerr << "Ошибка: указано больше одного входного файла\n";
                return false;
            }
            opt.inputFile = a;
        }
    }
    if (!opt.showHelp && opt.inputFile.empty()) {
        std::cerr << "Ошибка: не указан входной файл\n";
        return false;
    }
    return true;
}

static const char* lexName(int type) {
    static const char* names[L_EOF + 1] = {
        "?", "ID", "INT", "REAL", "ASSIGN",
        "PLUS", "MINUS", "MULTIPLICATION", "DIVISION", "LEFTPARENTHES",
        "RIGHTPARENTHES", "LEFTBRACKET", "RIGHTBRACKET", "COMMA", "SEMICOLON",
        "LESSERTHAN", "GREATERTHAN", "LESSEREQUAL", "GREATEREQUAL", "EQUAL",
        "NOTEQUAL", "IF", "THEN", "ELSE", "WHILE",
        "DO", "BEGIN", "END", "READ", "WRITE",
        "ARRAY", "EOF"
    };
    return (type >= 0 && type <= L_EOF)?names[type]:names[0];
}

static bool readFile(const std::string& path, std::string& text) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    text = ss.str();
    if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) text.erase(0, 3);
    return true;
}

static void reportError(const std::string& kind, int line, int col, const std::string& what, std::ostream& log, bool logIsFile) {
    std::ostringstream msg;
    msg << kind << ": строка " << line << ", символ " << col << ": " << what << "\n";
    std::cerr << msg.str();
    if (logIsFile) log << msg.str();
}

static void printLexemes(const std::string& text, const Options& opt, std::ostream& log) {
    Lexer lexer(text);
    int count = 0;
    log << "=== Лексический анализ ===\n";
    log << std::left << std::setw(10) << "Pos" << std::setw(5) << "No" << std::setw(18) << "Lex" << "Text\n";
    try {
        for (;;) {
            Lex l = lexer.next();
            count++;
            std::string pos = std::to_string(l.line) + ":" + std::to_string(l.col);
            log << std::left << std::setw(10) << pos << std::setw(5) << l.type << std::setw(18) << lexName(l.type) << l.value;
            if (opt.lexsVerbose) {
                if (l.type == L_INT) log << "    [int = " << l.intValue << "]";
                if (l.type == L_REAL) log << "    [real = " << l.realValue << "]";
            }
            log << "\n";
            if (l.type == L_EOF) break;
        }
        log << "Лексический анализ завершён: лексем " << count << " (включая EOF)\n";
    } catch (const LexError&) {
        log << "(список оборван лексической ошибкой, её описание - ниже)\n";
    }
}

static void printOps(const Parser& parser, bool verbose, std::ostream& log) {
    const Ops& ops = parser.ops();
    log << "=== ОПС (" << ops.size() << " элементов) ===\n";
    if (!verbose) {
        for (size_t i = 0; i < ops.size(); i++) {
            log << ((i)?" ":"") << parser.itemText(ops[i]);
        }
        log << "\n";
        return;
    }
    static const char* kinds[] = {"Var", "Arr", "Const", "Label", "Op"};
    log << std::left << std::setw(6) << "No" << std::setw(8) << "Kind" << std::setw(12) << "Item" << "Pos\n";
    for (size_t i = 0; i < ops.size(); i++) {
        const OpsItem& it = ops[i];
        log << std::left << std::setw(6) << i << std::setw(8) << kinds[(int)it.kind] << std::setw(12) << parser.itemText(it) << it.line << ":" << it.col << "\n";
    }
}

static void printTables(const Parser& parser, std::ostream& log) {
    log << "=== Таблицы имён ===\n";
    log << "Переменные (" << parser.variables().size() << "):";
    for (size_t i = 0; i < parser.variables().size(); i++) {
        log << " " << i << "=" << parser.variables()[i];
    }
    log << "\nМассивы (" << parser.arrays().size() << "):";
    for (size_t i = 0; i < parser.arrays().size(); i++) {
        log << " " << i << "=" << parser.arrays()[i];
    }
    log << "\n";
}

static bool runParser(Parser& parser, const Options& opt, std::ostream& log, bool logIsFile) {
    try {
        parser.parse();
    } catch (const LexError& e) {
        reportError("Лексическая ошибка", e.line, e.col, e.what(), log, logIsFile);
        return false;
    } catch (const ParseError& e) {
        reportError((e.semantic)?"Семантическая ошибка":"Синтаксическая ошибка", e.line, e.col, e.what(), log, logIsFile);
        return false;
    }
    if (opt.compileOnly || opt.showLexs || opt.showTrace || opt.showOps || opt.showTables) {
        log << "Синтаксический анализ завершён: элементов ОПС " << parser.ops().size() << "\n";
    }
    if (opt.showOps) printOps(parser, opt.opsVerbose, log);
    if (opt.showTables) printTables(parser, log);
    return true;
}

static bool runInterpreter(const Parser& parser, const Options& opt, std::ostream& log, bool logIsFile) {
    Interpreter interpreter(parser.ops(), parser.variables(), parser.arrays(), std::cin, std::cout, (opt.showExec)?&log:nullptr);
    bool ok = true;
    try {
        interpreter.run();
    } catch (const RuntimeError& e) {
        std::cout.flush();
        reportError("Ошибка выполнения", e.line, e.col, e.what(), log, logIsFile);
        ok = false;
    }
    if (opt.showMemory) interpreter.dumpMemory(log);
    return ok;
}

int main(int argc, char* argv[]) {
    Options opt;
    if (!parseArgs(argc, argv, opt)) {
        std::cerr << "\n";
        printUsage(std::cerr);
        return 1;
    }
    if (opt.showHelp) {
        printUsage(std::cout);
        return 0;
    }
    std::string text;
    if (!readFile(opt.inputFile, text)) {
        std::cerr << "Ошибка: не удалось открыть файл " << opt.inputFile << "\n";
        return 1;
    }
    std::ofstream logFile;
    bool logIsFile = !opt.logFile.empty();
    if (logIsFile) {
        logFile.open(opt.logFile);
        if (!logFile) {
            std::cerr << "Ошибка: не удалось создать файл логов " << opt.logFile << "\n";
            return 1;
        }
    }
    std::ostream& log = (logIsFile)?static_cast<std::ostream&>(logFile):std::cout;
    if (opt.showLexs) printLexemes(text, opt, log);
    Parser parser(Lexer(text), (opt.showTrace)?&log:nullptr);
    if (!runParser(parser, opt, log, logIsFile)) return 2;
    if (!opt.compileOnly && !runInterpreter(parser, opt, log, logIsFile)) return 2;
    return 0;
}