#include "cpplox/log/Log.hpp"
#include "cpplox/core/StringFormatter.hpp"
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
using cpplox::InterpretResultCode;
using cpplox::DiagnosticConsumer;
using cpplox::DiagnosticEngine;

class DiagnosticLogger : public DiagnosticConsumer {
public:
    void consume(cpplox::Diagnostic&& d) override {
        cpplox::errorln("Compile error on line {}: {}.", d.line, d.msg);
    }
};

bool isASCII(const std::string& str);

InterpretResult interpret(std::string source,
                          bool repl,
                          DiagnosticEngine& e,
                          VM& vm,
                          Compiler& c);
void repl(DiagnosticEngine& e, VM& vm, Compiler& c);
bool readFile(const char* filename, std::string& result);

int main(int argc, const char* argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));

    DiagnosticEngine diagnostics(std::make_unique<DiagnosticLogger>());
    Compiler compiler;
    VM vm;

    if (argc == 1) {
        repl(diagnostics, vm, compiler);
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

        const auto r = interpret(std::move(source), false, diagnostics, vm, compiler);
        if (r.code == InterpretResultCode::COMPILE_ERROR) {
            return 65;
        }
        if (r.code == InterpretResultCode::RUNTIME_ERROR) {
            cpplox::errorln("Runtime error: {}", r.error);
            return 70;
        }
    } else {
        return 64;
    }

    return 0;
}

void repl(DiagnosticEngine& e, VM& vm, Compiler& compiler) {
    std::string line = "";

    for (;;) {
        cpplox::print("> ");

        std::getline(std::cin, line);
        if (std::cin.fail() == false) {
            if (line == ":q") {
                break;
            }

            if (isASCII(line)) {
                const auto r = interpret(line, true, e, vm, compiler);
                if (r.code == InterpretResultCode::RUNTIME_ERROR) {
                    cpplox::errorln("Runtime error: {}", r.error);
                }
            } else {
                cpplox::errorln("Input error. Non-ascii charater found.");
            }
        } else {
            cpplox::errorln("Input error. Please try again.");
        }
    }
}

InterpretResult interpret(std::string source,
                          bool repl,
                          DiagnosticEngine& e,
                          VM& vm,
                          Compiler& compiler) {
    cpplox::CompileResult compiled;
    if (repl) {
        compiled = compiler.replExpression(source, nullptr);
        if (compiled.error) {
            compiled = compiler.compile(std::move(source), &e);
        }
    } else {
        compiled = compiler.compile(std::move(source), &e);
    }

    InterpretResult r;
    if (compiled.error) {
        r.code = InterpretResultCode::COMPILE_ERROR;
    } else {
        r = vm.interpret(compiled.function, std::move(compiled.gcObjects));
    }

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