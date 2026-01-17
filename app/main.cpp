#include "cpplox/log/Log.hpp"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/vm/VM.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using cpplox::VM;
using cpplox::Compiler;
using cpplox::InterpretResult;
using cpplox::DiagnosticConsumer;
using cpplox::DiagnosticEngine;

class DiagnosticLogger : public DiagnosticConsumer {
public:
    void consume(cpplox::Diagnostic&& d) override {
        cpplox::errorln("Compile error on line {}: {}.", d.line, d.msg);
    }
};

bool isASCII(const std::string& str);

InterpretResult interpret(std::string source, VM& vm, Compiler& c);
void repl(VM& vm, Compiler& c);
bool readFile(const char* filename, std::string& result);

int main(int argc, const char* argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));

    DiagnosticEngine diagnostics(std::make_unique<DiagnosticLogger>());
    Compiler compiler(&diagnostics);
    VM vm;

    if (argc == 1) {
        repl(vm, compiler);
    } else if (argc == 2) {
        std::string source = "";
        bool bFileOk = readFile(argv[1], source);
        if (bFileOk == false) {
            cpplox::errorln("Error reading '{}'", argv[1]);
            return 74;
        }
        if (isASCII(source) == false) {
            cpplox::errorln("Invalid input - '{}' - non-ascii characters found", argv[1]);
            return 1;
        }

        auto r = interpret(std::move(source), vm, compiler);
        if (r != InterpretResult::OK) {
            return 1;
        }
    } else {
        return 64;
    }

    return 0;
}

void repl(VM& vm, Compiler& compiler) {
    std::string line = "";

    for (;;) {
        cpplox::print("> ");

        std::getline(std::cin, line);
        if (std::cin.fail() == false) {
            if (line == ":q") {
                break;
            }

            if (isASCII(line)) {
                interpret(line, vm, compiler);
            } else {
                cpplox::errorln("Input error. Non-ascii charater found.");
            }
        } else {
            cpplox::errorln("Input error. Please try again.");
        }
    }
}

InterpretResult interpret(std::string source, VM& vm, Compiler& compiler) {
    cpplox::Function* func = nullptr;
    cpplox::Vector<cpplox::Object*> objs;
    bool hadError = compiler.compile(std::move(source), func, objs);
    if (hadError) {
        return InterpretResult::COMPILE_ERROR;
    }

    auto r = vm.interpret(func, std::move(objs));

    return r;
}

// Reads a file in a single step.
// We assume source files are not very big.
bool readFile(const char* filename, std::string& result) {
    std::ifstream file(filename);
    if (!file) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    result = buffer.str();

    file.close();
    return true;
}

bool isASCII(const std::string& str) {
    for (char c : str) {
        if (static_cast<unsigned char>(c) > 127) {
            return false;
        }
    }

    return true;
}