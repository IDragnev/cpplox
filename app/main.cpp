#include "cpplox/log/Log.hpp"
#include "cpplox/core/StringFormatter.hpp"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/vm/VM.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"

#include <cstdlib>
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
    explicit DiagnosticLogger(bool reportLines) : reportLines(reportLines) {}

    void consume(cpplox::Diagnostic&& d) override {
        if (reportLines) {
            cpplox::errorln("Compile error on line {}: {}.", d.line, d.msg);
        } else {
            cpplox::errorln("Compile error: {}.", d.msg);
        }
    }
    
private:
    bool reportLines;
};

void printRuntimeError(const cpplox::RuntimeError& err, bool reportLines) {
    const std::size_t TRACE_HEAD_FRAMES = 10;

    const auto printFrame = [reportLines](const cpplox::StackFrame& frame) {
        if (reportLines) {
            cpplox::error("\n[line {}] in {}", frame.line, frame.functionName);
        } else {
            cpplox::error("\nin {}", frame.functionName);
        }
    };

    cpplox::error("Runtime error: {}", err.msg);

    const std::size_t count = err.frames.getCount();
    const bool elide = count > TRACE_HEAD_FRAMES + 2;
    const std::size_t shown = elide ? TRACE_HEAD_FRAMES : count;

    for (std::size_t i = 0; i < shown; ++i) {
        printFrame(err.frames[i]);
    }

    if (elide) {
        cpplox::error("\n... {} frames omitted", count - shown - 1);
        printFrame(err.frames[count - 1]);
    }

    cpplox::errorln("");
}

bool hasEnvVar(const char* name) {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    return std::getenv(name) != nullptr;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

bool isASCII(const std::string& str);

InterpretResult interpret(std::string_view source,
                          bool repl,
                          DiagnosticEngine& e,
                          VM& vm,
                          Compiler& c);
void repl(DiagnosticEngine& e, VM& vm, Compiler& c);
bool readFile(const char* filename, std::string& result);

int main(int argc, const char* argv[]) {
    const bool replMode = (argc == 1);
    DiagnosticLogger logger(replMode == false);
    DiagnosticEngine diagnostics(&logger);
    Compiler compiler;
    compiler.setOptions({
        .forceLongInstructions = hasEnvVar("CPPLOX_FORCE_LONG_OPS"),
    });
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

        const auto r = interpret(source, false, diagnostics, vm, compiler);
        if (r.code == InterpretResultCode::COMPILE_ERROR) {
            return 65;
        }
        if (r.code == InterpretResultCode::RUNTIME_ERROR) {
            printRuntimeError(r.error, true);
            return 70;
        }
    } else {
        return 64;
    }

    return 0;
}

void repl(DiagnosticEngine& e, VM& vm, Compiler& compiler) {
    if (cpplox::STDIN_IS_TERMINAL) {
        cpplox::println("cpplox REPL. Type :q to exit.");
    }

    std::string line = "";

    for (;;) {
        if (cpplox::STDIN_IS_TERMINAL) {
            cpplox::print("> ");
        }

        std::getline(std::cin, line);
        if (std::cin.eof()) { break; }
        if (std::cin.fail()) {
            cpplox::errorln("Input error. Please try again.");
            std::cin.clear();
            continue;
        }

        if (line == ":q") {
            break;
        }

        while (!line.empty() && line.back() == '\\') {
            line.pop_back();
            if (cpplox::STDIN_IS_TERMINAL) {
                cpplox::print(".. ");
            }
            std::string next;
            std::getline(std::cin, next);
            if (std::cin.eof()) break;
            if (std::cin.fail()) break;
            line += next;
        }

        if (isASCII(line)) {
            const auto r = interpret(line, true, e, vm, compiler);
            if (r.code == InterpretResultCode::RUNTIME_ERROR) {
                printRuntimeError(r.error, false);
            }
        } else {
            cpplox::errorln("Input error. Non-ascii charater found.");
        }
    }
}

InterpretResult interpret(std::string_view source,
                          bool repl,
                          DiagnosticEngine& e,
                          VM& vm,
                          Compiler& compiler) {
    cpplox::CompileResult compiled;
    if (repl) {
        compiled = compiler.replExpression(source, nullptr);
        if (compiled.error) {
            compiled = compiler.compile(source, &e);
        }
    } else {
        compiled = compiler.compile(source, &e);
    }

    InterpretResult r;
    if (compiled.error) {
        r.code = InterpretResultCode::COMPILE_ERROR;
    } else {
        r = vm.interpret(compiled.function, compiled.gcObjects);
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