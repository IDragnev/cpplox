#include "cpplox/core/Compiler.hpp"

namespace cpplox {
    Compiler::Compiler()
        : source("")
        , scanner(source)
    {
    }

    CompileResult Compiler::compile(std::string compileSource, Chunk& chunk) {
        init(std::move(compileSource), chunk);

        // real work start
        advance();
        expression();
        // real work end

        CompileResult result;
        result.errors = std::move(this->errors);
        result.hasError = parser.hadError;

        cleanUp();

        return result;
    }

    void Compiler::init(std::string compileSource, Chunk& chunk) {
        source = std::move(compileSource);
        scanner = Scanner(source);
        currentChunk = &chunk;
    }

    void Compiler::cleanUp() {
        source = "";
        scanner = Scanner(source);
        parser = Parser{};
        errors = std::vector<CompileError>{};
        currentChunk = nullptr;
    }

    void Compiler::advance() {
        parser.previous = parser.current;

        while (scanner.isDone() == false) {
            ScanResult r = scanner.scanToken();
            parser.current = r.token;

            if (r.errorType == ScanError::OK) {
                break;
            }
            else {
                addError(CompileError{
                    .type = CompileErrorType::SCAN_ERROR,
                    .scanError = r.errorType,
                    .token = parser.current,
                });
            }
        }
    }

    void Compiler::expression() {}

    bool Compiler::consumeToken(TokenType tokenType) {
        if (parser.current.type == tokenType) {
            advance();
            return true;
        }

        addError(CompileError{
            .type = CompileErrorType::EXPECTED_TOKEN,
            .token = parser.current,
            .expectedToken = tokenType,
        });

        return false;
    }

    void Compiler::addError(const CompileError& e) {
        if (parser.panicMode) {
            return;
        }

        parser.panicMode = true;
        parser.hadError = true;
        errors.push_back(e);
    }
}