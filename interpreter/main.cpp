#include "cpplox/core/VM.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using cpplox::VM;
using cpplox::InterpretResult;

InterpretResult interpret(const std::string_view& source, VM& vm) {
    (void)vm;
    printf("input = %s\n", source.data());

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

            interpret(line, vm);
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
    VM vm;

    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        std::string source = "";
        bool bFileOk = readFile(argv[1], source);
        if (bFileOk == false) {
            fprintf(stderr, "Error reading %s\n", argv[1]);
            return 74;
        }

        auto result = interpret(source, vm);
        (void)result;
    } else {
        return 64;
    }

    return 0;
}