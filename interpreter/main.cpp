#include "cpplox/core/VM.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "cpplox/core/Compiler.hpp"

using cpplox::VM;
using cpplox::InterpretResult;

bool isASCII(const std::string& str) {
    for (char c : str) {
        if (static_cast<unsigned char>(c) > 127) {
            return false;
        }
    }

    return true;
}

InterpretResult interpret(std::string source, VM& vm) {
    printf("input = %s\n", source.data());

    cpplox::Compiler compiler;
    cpplox::Chunk chunk;

    cpplox::CompileResult r = compiler.compile(std::move(source), chunk);
    if (r.hasError) {
        // log message based on error type
        return InterpretResult::COMPILE_ERROR;
    }

    (void)vm;
    //return vm.interpret(chunk);
    return InterpretResult::OK;
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
                printf("Input error. Non-ascii charater found.\n");
            }
        } else {
            printf("Input error. Please try again.\n");
        }
    }
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

        auto result = interpret(std::move(source), vm);
        (void)result;
    } else {
        return 64;
    }

    return 0;
}