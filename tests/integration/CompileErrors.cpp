#include "doctest/doctest.h"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"
#include "cpplox/diagnostics/Diagnostic.hpp"

#include <string>
#include <string_view>
#include <vector>

class DiagnosticsCollector : public cpplox::DiagnosticConsumer {
public:
    std::vector<cpplox::Diagnostic> diagnostics;

    void consume(cpplox::Diagnostic&& d) override {
        diagnostics.push_back(std::move(d));
    }
};

std::vector<cpplox::Diagnostic> compileAndCollect(std::string_view src) {
    DiagnosticsCollector collector;
    cpplox::DiagnosticEngine engine(&collector);
    cpplox::Compiler compiler;

    compiler.compile(src, &engine);

    return std::move(collector.diagnostics);
}

TEST_CASE("break outside loop") {
    const auto diags = compileAndCollect("break;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'break' outside of loop");
    CHECK(diags[0].line == 1);
}

TEST_CASE("continue outside loop") {
    const auto diags = compileAndCollect("continue;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'continue' outside of loop");
    CHECK(diags[0].line == 1);
}

TEST_CASE("break resets across function boundary") {
    const auto diags = compileAndCollect(R"(while (true) {
    fun f() {
        break;
    }
})");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'break' outside of loop");
    CHECK(diags[0].line == 3);
}

TEST_CASE("this outside class") {
    const auto diags = compileAndCollect("var x = this;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'this' outside of class");
    CHECK(diags[0].line == 1);
}

TEST_CASE("super outside class") {
    const auto diags = compileAndCollect("super.method();");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'super' outside of a class");
    CHECK(diags[0].line == 1);
}

TEST_CASE("super in class with no superclass") {
    const auto diags = compileAndCollect(R"(class A {
    method() {
        super.method();
    }
})");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't use 'super' in a class with no superclass");
    CHECK(diags[0].line == 3);
}

TEST_CASE("class inherits from itself") {
    const auto diags = compileAndCollect("class A < A {}");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "A class can't inherit from itself");
    CHECK(diags[0].line == 1);
}

TEST_CASE("return from top-level") {
    const auto diags = compileAndCollect("return;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't return from top-level code");
    CHECK(diags[0].line == 1);
}

TEST_CASE("return value from initializer") {
    const auto diags = compileAndCollect(R"(class A {
    init() {
        return 42;
    }
})");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't return a value from an initializer");
    CHECK(diags[0].line == 3);
}

TEST_CASE("duplicate variable in same scope") {
    const auto diags = compileAndCollect(R"({
    var a = 1;
    var a = 2;
})");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Variable with name 'a' already exists in this scope");
    CHECK(diags[0].line == 3);
}

TEST_CASE("read local in own initializer") {
    const auto diags = compileAndCollect(R"({
    var a = a;
})");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't read a local variable in its initializer");
    CHECK(diags[0].line == 2);
}

TEST_CASE("invalid assignment target") {
    const auto diags = compileAndCollect("1 + 2 = 3;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Invalid assignment target");
    CHECK(diags[0].line == 1);
}

TEST_CASE("expected expression") {
    const auto diags = compileAndCollect("var x = ;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Expected expression");
    CHECK(diags[0].line == 1);
}

TEST_CASE("expected expression at end of input") {
    const auto diags = compileAndCollect("var a =");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Expected expression");
    CHECK(diags[0].line == 1);
}

// A literal needs a digit on both sides of its dot. Neither form is a lexical
// error though: the scanner ends the number at the dot, so a trailing one is
// parsed as the start of a property access and a leading one as no expression
// at all.
TEST_CASE("number literal with a dot on one side only") {
    const auto trailing = compileAndCollect("var a = 1.;");
    const auto trailingFraction = compileAndCollect("var a = 1.5.;");
    const auto leading = compileAndCollect("var a = .5;");

    REQUIRE(trailing.size() == 1);
    CHECK(trailing[0].msg == "Expected property name after '.'");
    CHECK(trailing[0].line == 1);

    REQUIRE(trailingFraction.size() == 1);
    CHECK(trailingFraction[0].msg == "Expected property name after '.'");

    REQUIRE(leading.size() == 1);
    CHECK(leading[0].msg == "Expected expression");

    const auto afterExponent = compileAndCollect("var a = 2.5e2.5;");
    REQUIRE(afterExponent.size() == 1);
    CHECK(afterExponent[0].msg == "Expected property name after '.'");

    CHECK(compileAndCollect("var a = 1.0;").empty());
    CHECK(compileAndCollect("var a = 2.5e2;").empty());
}

TEST_CASE("number literal too large") {
    const auto diags = compileAndCollect("var a = 1e400;");

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Number is too large");
    CHECK(diags[0].line == 1);

    // The largest double is just under 1.8e308, so this one still fits.
    CHECK(compileAndCollect("var a = 1e308;").empty());

    // Underflow leaves zero rather than an infinity, so it is accepted.
    CHECK(compileAndCollect("var a = 1e-400;").empty());
}

TEST_CASE("parameters limit") {
    std::string src = "fun f(";
    for (int i = 0; i < 256; ++i) {
        if (i > 0) src += ", ";
        src += "a" + std::to_string(i);
    }
    src += ") {}";

    const auto diags = compileAndCollect(src);

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't have more than 255 parameters");
    CHECK(diags[0].line == 1);
}

TEST_CASE("arguments limit") {
    std::string src = "f(";
    for (int i = 0; i < 256; ++i) {
        if (i > 0) src += ", ";
        src += std::to_string(i);
    }
    src += ");";

    const auto diags = compileAndCollect(src);

    REQUIRE(diags.size() == 1);
    CHECK(diags[0].msg == "Can't have more than 255 arguments");
    CHECK(diags[0].line == 1);
}

TEST_CASE("constants limit") {
    std::string src;
    for (int i = 0; i <= 65536; ++i) {
        src += std::to_string(i) + ";\n";
    }

    const auto diags = compileAndCollect(src);

    REQUIRE(diags.size() >= 1);
    CHECK(diags[0].msg == "Constants limits reached");
}

TEST_CASE("upvalues limit") {
    std::string src = "fun outer() {\n";
    for (int i = 0; i < 256; ++i) {
        src += "    var v" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    }
    src += "    fun inner() {\n";
    for (int i = 0; i < 256; ++i) {
        src += "        v" + std::to_string(i) + ";\n";
    }
    src += "    }\n";
    src += "}\n";

    const auto diags = compileAndCollect(src);

    REQUIRE(diags.size() >= 1);
    CHECK(diags[0].msg == "Can't have more than 255 captures in a closure");
}

TEST_CASE("locals limit" * doctest::skip()) {
    std::string src = "fun f() {\n";
    for (int i = 0; i <= 65536; ++i) {
        src += "    var v" + std::to_string(i) + " = 0;\n";
    }
    src += "}\n";

    const auto diags = compileAndCollect(src);

    REQUIRE(diags.size() >= 1);
    CHECK(diags[0].msg ==
          "Can't have more than 65536 local variables in a function");
}

TEST_CASE("multi-diagnostic recovery") {
    const auto diags = compileAndCollect(R"(( +
break;
( +
continue;)");

    REQUIRE(diags.size() == 4);
    CHECK(diags[0].msg == "Expected expression");
    CHECK(diags[0].line == 1);
    CHECK(diags[1].msg == "Can't use 'break' outside of loop");
    CHECK(diags[1].line == 2);
    CHECK(diags[2].msg == "Expected expression");
    CHECK(diags[2].line == 3);
    CHECK(diags[3].msg == "Can't use 'continue' outside of loop");
    CHECK(diags[3].line == 4);
}

TEST_CASE("null source") {
    cpplox::Compiler compiler;
    auto result = compiler.compile(std::string_view{}, nullptr);

    CHECK(result.error == true);
    CHECK(result.function == nullptr);
}
