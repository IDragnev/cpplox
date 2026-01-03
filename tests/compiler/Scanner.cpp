#include "doctest/doctest.h"
#include "cpplox/compiler/Scanner.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"

#include <vector>

class DiagnosticsIgnore : public cpplox::DiagnosticConsumer {
public:
    void consume(cpplox::Diagnostic&&) override {}
};

cpplox::DiagnosticEngine diag(std::make_unique<DiagnosticsIgnore>());

std::vector<cpplox::Token> scanAll(cpplox::Scanner& s) {
    std::vector<cpplox::Token> result;

    while (s.isDone() == false) {
        cpplox::ScanResult r = s.scanToken();
        if (r.error == false) {
            result.push_back(r.token);
        }
    }

    return result;
}

TEST_CASE("Scanning empty source") {
    std::string source = "";
    cpplox::Scanner scanner(source, &diag);

    CHECK(scanner.isDone());

    cpplox::ScanResult r = scanner.scanToken();
    CHECK(r.error);
    CHECK(r.token.type == cpplox::TokenType::EOF_TOKEN);
}

TEST_CASE("Scanning simple tokens") {
    std::string source = ". ; < - + x ( ) { } >= <= ==";
    cpplox::Scanner scanner(source, &diag);

    auto tokens = scanAll(scanner);

    CHECK(tokens.size() == 13);
    CHECK(scanner.isDone());
}

TEST_CASE("Scanning complex tokens") {
    std::string source = "\"str\" 123 123.456 123.";
    cpplox::Scanner scanner(source, &diag);

    cpplox::ScanResult r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::STRING);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::DOT);
}

TEST_CASE("Scanner handles invalid token") {
    std::string source = "& >";
    cpplox::Scanner scanner(source, &diag);

    cpplox::ScanResult r = scanner.scanToken();
    CHECK(r.error);
    CHECK(r.token.type == cpplox::TokenType::ERROR);

    CHECK(scanner.isDone() == false);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::GREATER);
}

TEST_CASE("Scanning unterminated string fails") {
    std::string source = "\"I am not terminated\n >";
    cpplox::Scanner scanner(source, &diag);

    cpplox::ScanResult r = scanner.scanToken();
    CHECK(r.error);
    CHECK(r.token.type == cpplox::TokenType::ERROR);

    CHECK(scanner.isDone());
}

TEST_CASE("Scanner ignores comments and whitespace") {
    std::string source = "//comment\n\t\r //comment\n >";
    cpplox::Scanner scanner(source, &diag);

    auto tokens = scanAll(scanner);

    CHECK(tokens.size() == 1);
    CHECK(scanner.isDone());
}

TEST_CASE("Scanner recognizes keywords") {
    std::string source = "class classx clas if else";
    cpplox::Scanner scanner(source, &diag);

    cpplox::ScanResult r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::CLASS);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::IDENTIFIER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::IDENTIFIER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::IF);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::ELSE);
}