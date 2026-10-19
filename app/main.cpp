#include "cpplox/log/Log.hpp"
#include "cpplox/core/StringFormatter.hpp"
#include "cpplox/core/Vector.hpp"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/vm/VM.hpp"
#include "cpplox/prelude/Prelude.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"

#include <cstdlib>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>

using cpplox::VM;
using cpplox::Compiler;
using cpplox::InterpretResult;
using cpplox::InterpretResultCode;
using cpplox::DiagnosticConsumer;
using cpplox::DiagnosticEngine;

constexpr std::string_view USAGE =
    "Usage: cpplox [script]\n"
    "       cpplox --version\n"
    "       cpplox --help\n"
    "\n"
    "Runs the given Lox script, or starts the REPL when given none.\n"
    "Type :q to leave the REPL.\n";

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

    const auto fromScript = [](const cpplox::StackFrame& frame) {
        return frame.source.size() == 0;
    };

    // Users can't see the prelude, so each run of prelude frames is shown as the
    // prelude function they called, without a line.
    cpplox::Vector<const cpplox::StackFrame*> frames;
    const std::size_t frameCount = err.frames.getCount();
    for (std::size_t i = 0; i < frameCount;) {
        if (!fromScript(err.frames[i])) {
            while (i + 1 < frameCount && !fromScript(err.frames[i + 1])) {
                ++i;
            }
        }
        frames.insertBack(&err.frames[i]);
        ++i;
    }

    const auto printFrame = [reportLines, &fromScript](const cpplox::StackFrame& frame) {
        if (reportLines && fromScript(frame)) {
            cpplox::error("\n[line {}] in {}", frame.line, frame.functionName);
        } else if (reportLines) {
            cpplox::error("\n[{}] in {}", frame.source, frame.functionName);
        } else if (fromScript(frame)) {
            cpplox::error("\nin {}", frame.functionName);
        } else {
            cpplox::error("\nin {} ({})", frame.functionName, frame.source);
        }
    };

    cpplox::error("Runtime error: {}", err.msg);

    const std::size_t count = frames.getCount();
    const bool elide = count > TRACE_HEAD_FRAMES + 2;
    const std::size_t shown = elide ? TRACE_HEAD_FRAMES : count;

    for (std::size_t i = 0; i < shown; ++i) {
        printFrame(*frames[i]);
    }

    if (elide) {
        cpplox::error("\n... {} frames omitted", count - shown - 1);
        printFrame(*frames[count - 1]);
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
bool loadPrelude(DiagnosticEngine& e, VM& vm, Compiler& c);
void repl(DiagnosticEngine& e, VM& vm, Compiler& c);
bool readFile(const char* filename, std::string& result);

int main(int argc, const char* argv[]) {
    if (argc > 2) {
        cpplox::errorln("Expected at most one argument.");
        cpplox::error("{}", USAGE);
        return 64;
    }

    if (argc == 2) {
        const std::string_view arg = argv[1];
        if (arg == "--version") {
            cpplox::println("cpplox {}", CPPLOX_VERSION);
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            cpplox::print("{}", USAGE);
            return 0;
        }
        if (arg.starts_with('-')) {
            cpplox::errorln("Unknown option '{}'.", arg);
            cpplox::error("{}", USAGE);
            return 64;
        }
    }

    const bool replMode = (argc < 2);
    DiagnosticLogger logger(replMode == false);
    DiagnosticEngine diagnostics(&logger);
    Compiler compiler;
    compiler.setOptions({
        .forceLongInstructions = hasEnvVar("CPPLOX_FORCE_LONG_OPS"),
    });
    VM vm;

    if (loadPrelude(diagnostics, vm, compiler) == false) {
        cpplox::errorln("Internal error: the prelude failed to load.");
        return 70;
    }

    if (replMode) {
        repl(diagnostics, vm, compiler);
    } else {
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

bool loadPrelude(DiagnosticEngine& e, VM& vm, Compiler& compiler) {
    const auto compiled = compiler.compile(cpplox::PRELUDE_SOURCE, &e, cpplox::PRELUDE_NAME);
    if (compiled.error) {
        return false;
    }

    const auto r = vm.interpret(compiled.function, compiled.gcObjects);
    if (r.code == InterpretResultCode::RUNTIME_ERROR) {
        printRuntimeError(r.error, true);
        return false;
    }

    return true;
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