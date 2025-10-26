#include "cpplox/core/VM.hpp"
#include "cpplox/core/Compiler.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using cpplox::VM;
using cpplox::InterpretResult;

void logCompileErrors(const cpplox::CompileResult& r);
void logCompileError(const cpplox::CompileError& r);

bool isASCII(const std::string& str);

InterpretResult interpret(std::string source, VM& vm);
void repl(VM& vm);
bool readFile(const char* filename, std::string& result);

int main(int argc, const char* argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));

    VM vm;

    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        std::string source = "";
        bool bFileOk = readFile(argv[1], source);
        if (bFileOk == false) {
            fprintf(stderr, "Error reading '%s'\n", argv[1]);
            return 74;
        }
        if (isASCII(source) == false) {
            fprintf(stderr, "Invalid input - '%s' - non-ascii characters found\n", argv[1]);
            return 1;
        }

        auto r = interpret(std::move(source), vm);
        if (r != InterpretResult::OK) {
            return 1;
        }
    } else {
        return 64;
    }

    return 0;
}

void repl(VM& vm) {
    std::string line = "";

    for (;;) {
        printf("> ");

        std::getline(std::cin, line);
        if (std::cin.fail() == false) {
            if (line == ":q") {
                break;
            }

            if (isASCII(line)) {
                interpret(line, vm);
            } else {
                fprintf(stderr, "Input error. Non-ascii charater found.\n");
            }
        } else {
            fprintf(stderr, "Input error. Please try again.\n");
        }
    }
}

InterpretResult interpret(std::string source, VM& vm) {
    cpplox::Compiler compiler;
    cpplox::Chunk chunk;

    cpplox::CompileResult r = compiler.compile(std::move(source), chunk);
    if (r.hasError) {
        logCompileErrors(r);
        return InterpretResult::COMPILE_ERROR;
    }

    return vm.interpret(chunk);
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

void logCompileErrors(const cpplox::CompileResult& r) {
    fprintf(stderr, "Compile error.\n");

    for (const auto& e : r.errors) {
        logCompileError(e);
    }
}

void logCompileError(const cpplox::CompileError& e) {
    using cpplox::CompileErrorType;
    using cpplox::ScanError;

    fprintf(stderr, "Error at line %u: ", e.token.line);

    switch (e.type) {
        case CompileErrorType::SCAN_ERROR: {
            switch (e.scanError) {
                case ScanError::UNKNOWN_CHARACTER: {
                    fprintf(stderr, "Unknown character found.");
                } break;
                case ScanError::UNTERMINATED_STRING: {
                    fprintf(stderr, "Unterminated string.");
                } break;
                default: break;
            }
        } break;
        case CompileErrorType::EXPECTED_TOKEN: {
            fprintf(stderr,
                    "Expected token %d.",
                    static_cast<int>(e.expectedToken));
            if (e.token.type != cpplox::TokenType::ERROR) {
                fprintf(stderr, "Found %d.", static_cast<int>(e.token.type));
            }
        } break;
        case CompileErrorType::EXPECTED_EXPRESSION: {
            fprintf(stderr, "Expected expression.");
        } break;
        case CompileErrorType::EXPECTED_STATEMENT: {
            fprintf(stderr, "Expected statement.");
        } break;
        case CompileErrorType::CONSTANTS_LIMIT_REACHED: {
            fprintf(stderr, "Constants limit reached!");
        } break;
        default: break;
    }

    fprintf(stderr, "\n");
}