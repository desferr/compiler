#include "lexer.h"
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
};

static void printUsage(std::ostream& out) {
    out << "Использование: compiler.exe [флаги] <файл>\n"
        << "Флаги:\n"
        << "    -h, --help  показать эту справку\n"
        << "    -l <файл>   вывод логов работы в файл\n"
        << "    -t          вывод списка лексем\n"
        << "    -tv         вывод списка лексем с подробностями\n"
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

static bool runLexer(const std::string& text, const Options& opt, std::ostream& log, bool logIsFile) {
    Lexer lexer(text);
    int count = 0;
    if (opt.showLexs) {
        log << "=== Лексический анализ ===\n";
        log << std::left << std::setw(10) << "Pos" << std::setw(5) << "No" << std::setw(18) << "Lex" << "Text\n";
    }
    try {
        for (;;) {
            Lex l = lexer.next();
            count++;
            if (opt.showLexs) {
                std::string pos = std::to_string(l.line) + ":" + std::to_string(l.col);
                log << std::left << std::setw(10) << pos << std::setw(5) << l.type << std::setw(18) << lexName(l.type) << l.value;
                if (opt.lexsVerbose) {
                    if (l.type == L_INT) log << "    [int = " << l.int_value << "]";
                    if (l.type == L_REAL) log << "    [real = " << l.real_value << "]";
                }
                log << "\n";
            }
            if (l.type == L_EOF) break;
        }
    } catch (const LexError& e) {
        std::ostringstream msg;
        msg << "Лексическая ошибка: строка " << e.line << ", символ " << e.col << ": " << e.what() << "\n";
        std::cerr << msg.str();
        if (logIsFile) log << msg.str();
        return false;
    }
    log << "Лексический анализ завершён: лексем " << count << " (включая EOF)\n";
    return true;
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
    if (!runLexer(text, opt, log, logIsFile)) return 2;
    return 0;
}