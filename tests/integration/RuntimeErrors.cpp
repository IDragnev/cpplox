#include "doctest/doctest.h"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/vm/VM.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"
#include "cpplox/diagnostics/Diagnostic.hpp"

using cpplox::String;
using cpplox::InterpretResultCode;

class DiagnosticsIgnore : public cpplox::DiagnosticConsumer {
public:
    void consume(cpplox::Diagnostic&&) override {}
};

cpplox::InterpretResult runAndCollect(const char* src) {
    DiagnosticsIgnore ignore;
    cpplox::DiagnosticEngine engine(&ignore);
    cpplox::Compiler compiler;
    cpplox::VM vm;

    auto compiled = compiler.compile(src, &engine);
    REQUIRE(compiled.function != nullptr);

    return vm.interpret(compiled.function, std::move(compiled.gcObjects));
}

TEST_CASE("undefined variable") {
    const auto r = runAndCollect("print(x);");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Undefined variable 'x'."));
}

TEST_CASE("unary operand type error") {
    const auto r = runAndCollect("var a = -\"not a number\";");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Operand must be a number."));
}

TEST_CASE("binary operand type error - add") {
    const auto r = runAndCollect("var a = \"str\" + 1;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Operands must be two numbers or two strings."));
}

TEST_CASE("binary operand type error - subtract") {
    const auto r = runAndCollect("var a = \"a\" - 1;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Operands must be numbers."));
}

TEST_CASE("call nil") {
    const auto r = runAndCollect("nil();");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only call functions and classes."));
}

TEST_CASE("call bool") {
    const auto r = runAndCollect("true();");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only call functions and classes."));
}

TEST_CASE("call number") {
    const auto r = runAndCollect("123();");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only call functions and classes."));
}

TEST_CASE("call instance") {
    const auto r = runAndCollect(R"(class Foo {}
var foo = Foo();
foo();)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only call functions and classes."));
}

TEST_CASE("property get on non-instance") {
    const auto r = runAndCollect("nil.foo;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Only instances have properties."));
}

TEST_CASE("property set on non-instance") {
    const auto r = runAndCollect("nil.foo = \"value\";");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Only instances have fields."));
}

TEST_CASE("inherit non-class") {
    const auto r = runAndCollect(R"(var NotAClass = "not a class";
class Foo < NotAClass {})");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only inherit classes."));
}

TEST_CASE("assert failed") {
    const auto r = runAndCollect("assert(1 == 2);");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Assertion failed."));
}

TEST_CASE("assert with custom message") {
    const auto r = runAndCollect(
        R"(var x = 3;
assert(x == 2, "expected 2, got " + str(x));)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("expected 2, got 3"));
}

TEST_CASE("assert argument count") {
    const auto r = runAndCollect("assert();");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Invalid argument count. Expected 1 to 2, found 0."));
}

TEST_CASE("native argument count") {
    const auto r = runAndCollect("clock(1);");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Invalid argument count. Expected 0, found 1."));
}

TEST_CASE("shadowed print native") {
    const auto r = runAndCollect(R"(var print = 10;
print(1);)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Can only call functions and classes."));
}

TEST_CASE("undefined property") {
    const auto r = runAndCollect(R"(class Foo {}
var f = Foo();
f.noSuchProp;)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Undefined property 'noSuchProp'."));
}

TEST_CASE("undefined method via invoke") {
    const auto r = runAndCollect(R"(class Foo {}
var f = Foo();
f.noSuchMethod();)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Undefined property 'noSuchMethod'."));
}

TEST_CASE("wrong argument count for function") {
    const auto r = runAndCollect(R"(fun f(a) {}
f(1, 2);)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Invalid argument count. Expected 1, found 2."));
}

TEST_CASE("wrong argument count for class without init") {
    const auto r = runAndCollect(R"(class Foo {}
Foo(1);)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Expected 0 arguments but got 1."));
}

TEST_CASE("nested call stack trace") {
    const auto r = runAndCollect(R"(fun c() {
    nil.field;
}
fun b() {
    c();
}
fun a() {
    b();
}
a();)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(r.error.msg == String("Only instances have properties."));

    REQUIRE(r.error.frames.getCount() == 4);
    CHECK(r.error.frames[0].functionName == String("c"));
    CHECK(r.error.frames[0].line == 2);
    CHECK(r.error.frames[1].functionName == String("b"));
    CHECK(r.error.frames[1].line == 5);
    CHECK(r.error.frames[2].functionName == String("a"));
    CHECK(r.error.frames[2].line == 8);
    CHECK(r.error.frames[3].functionName == String("script"));
    CHECK(r.error.frames[3].line == 10);
}

TEST_CASE("top-level error has single script frame") {
    const auto r = runAndCollect("nil.field;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(r.error.msg == String("Only instances have properties."));
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.frames[0].functionName == String("script"));
    CHECK(r.error.frames[0].line == 1);
}

TEST_CASE("method call stack trace") {
    const auto r = runAndCollect(R"(class Foo {
    bar() {
        nil.field;
    }
}
Foo().bar();)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(r.error.msg == String("Only instances have properties."));
    REQUIRE(r.error.frames.getCount() == 2);
    CHECK(r.error.frames[0].functionName == String("bar"));
    CHECK(r.error.frames[0].line == 3);
    CHECK(r.error.frames[1].functionName == String("script"));
    CHECK(r.error.frames[1].line == 6);
}
