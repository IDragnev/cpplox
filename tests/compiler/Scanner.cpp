#include "doctest/doctest.h"
#include "cpplox/compiler/Scanner.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"

#include <vector>
#include <ostream>

class DiagnosticsIgnore : public cpplox::DiagnosticConsumer {
public:
    void consume(cpplox::Diagnostic&&) override {}
};

DiagnosticsIgnore ignore;
cpplox::DiagnosticEngine diag(&ignore);

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
    std::string source = "\"str\" 123 123.456 1e5 1.5e-3 1E+16 123.";
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
    CHECK(r.token.lexeme == "1e5");

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);
    CHECK(r.token.lexeme == "1.5e-3");

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);
    CHECK(r.token.lexeme == "1E+16");

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::NUMBER);

    r = scanner.scanToken();
    CHECK_FALSE(r.error);
    CHECK(r.token.type == cpplox::TokenType::DOT);
}

TEST_CASE("Scanning an e that is not an exponent") {
    std::string source = "1e 3ex 2e+";
    cpplox::Scanner scanner(source, &diag);

    auto tokens = scanAll(scanner);

    REQUIRE(tokens.size() == 7);
    CHECK(tokens[0].type == cpplox::TokenType::NUMBER);
    CHECK(tokens[0].lexeme == "1");
    CHECK(tokens[1].type == cpplox::TokenType::IDENTIFIER);
    CHECK(tokens[1].lexeme == "e");

    CHECK(tokens[2].type == cpplox::TokenType::NUMBER);
    CHECK(tokens[2].lexeme == "3");
    CHECK(tokens[3].type == cpplox::TokenType::IDENTIFIER);
    CHECK(tokens[3].lexeme == "ex");

    CHECK(tokens[4].type == cpplox::TokenType::NUMBER);
    CHECK(tokens[4].lexeme == "2");
    CHECK(tokens[5].type == cpplox::TokenType::IDENTIFIER);
    CHECK(tokens[5].lexeme == "e");
    CHECK(tokens[6].type == cpplox::TokenType::PLUS);
}

TEST_CASE("Scanning a fractional exponent ends the number at the dot") {
    std::string source = "2.5e2.5";
    cpplox::Scanner scanner(source, &diag);

    auto tokens = scanAll(scanner);

    REQUIRE(tokens.size() == 3);
    CHECK(tokens[0].type == cpplox::TokenType::NUMBER);
    CHECK(tokens[0].lexeme == "2.5e2");
    CHECK(tokens[1].type == cpplox::TokenType::DOT);
    CHECK(tokens[2].type == cpplox::TokenType::NUMBER);
    CHECK(tokens[2].lexeme == "5");
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