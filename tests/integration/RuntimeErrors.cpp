#include "doctest/doctest.h"
#include "cpplox/compiler/Compiler.hpp"
#include "cpplox/vm/VM.hpp"
#include "cpplox/diagnostics/DiagnosticEngine.hpp"
#include "cpplox/diagnostics/Diagnostic.hpp"

#include <string>

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
    CHECK(r.error.msg == String("Operands must be two numbers, two strings or two lists."));
}

TEST_CASE("binary operand type error - subtract") {
    const auto r = runAndCollect("var a = \"a\" - 1;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Operands must be numbers."));
}

TEST_CASE("binary operand type error - multiply") {
    const auto r = runAndCollect("var a = \"a\" * 2;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("Operands must be numbers."));
}

TEST_CASE("binary operand type error - compare") {
    const auto less = runAndCollect("var a = 1 < \"b\";");
    const auto greaterEqual = runAndCollect("var a = nil >= 1;");

    REQUIRE(less.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(greaterEqual.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(less.error.msg == String("Operands must be numbers."));
    CHECK(greaterEqual.error.msg == String("Operands must be numbers."));
}

// Divide checks its operand types before it checks the divisor, so a
// non-number divisor reports the type rather than the zero.
TEST_CASE("binary operand type error - divide") {
    const auto right = runAndCollect("var a = \"a\" / 2;");
    const auto left = runAndCollect("var a = 2 / \"a\";");
    const auto both = runAndCollect("var a = \"a\" / 0;");

    REQUIRE(right.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(left.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(both.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(right.error.msg == String("Operands must be numbers."));
    CHECK(left.error.msg == String("Operands must be numbers."));
    CHECK(both.error.msg == String("Operands must be numbers."));
}

TEST_CASE("division by zero") {
    const auto literal = runAndCollect("var a = 1 / 0;");
    const auto computed = runAndCollect("var b = 5 - 5;\nvar a = 1 / b;");
    const auto negativeZero = runAndCollect("var a = 1 / -0;");
    const auto zeroOverZero = runAndCollect("var a = 0 / 0;");

    REQUIRE(literal.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(computed.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(negativeZero.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(zeroOverZero.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(literal.error.frames.getCount() == 1);
    CHECK(literal.error.msg == String("Division by zero."));
    CHECK(computed.error.msg == String("Division by zero."));
    CHECK(negativeZero.error.msg == String("Division by zero."));
    CHECK(zeroOverZero.error.msg == String("Division by zero."));
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

TEST_CASE("stack overflow: wide frames exhaust the slot budget first") {
    std::string wideSrc = "fun f(n) {\n";
    for (int i = 0; i < 300; ++i) {
        wideSrc += "    var v" + std::to_string(i) + " = 0;\n";
    }
    wideSrc += "    return f(n + 1);\n}\nf(0);";

    const auto deep = runAndCollect(R"(fun f(n) {
    return f(n + 1);
}
f(0);)");
    const auto wide = runAndCollect(wideSrc.c_str());

    REQUIRE(deep.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(wide.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(deep.error.msg == String("Stack overflow."));
    CHECK(wide.error.msg == String("Stack overflow."));

    CHECK(wide.error.frames.getCount() < deep.error.frames.getCount());
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

TEST_CASE("subscript on a non-list") {
    const auto str = runAndCollect("\"abc\"[0];");
    const auto nil = runAndCollect("nil[0];");
    const auto number = runAndCollect("var x = 1;\nx[0] = 2;");
    const auto instance = runAndCollect("class Foo {}\nFoo()[0];");

    REQUIRE(str.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(nil.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(number.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(instance.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(str.error.frames.getCount() == 1);
    CHECK(str.error.msg == String("Only lists can be indexed, got string."));
    CHECK(nil.error.msg == String("Only lists can be indexed, got nil."));
    CHECK(number.error.msg == String("Only lists can be indexed, got number."));
    CHECK(instance.error.msg == String("Only lists can be indexed, got instance."));
}

TEST_CASE("list index is not a number") {
    const auto str = runAndCollect("[1][\"0\"];");
    const auto boolean = runAndCollect("[1][true];");
    const auto nil = runAndCollect("[1][nil] = 2;");
    const auto list = runAndCollect("[1][[0]];");

    REQUIRE(str.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(boolean.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(nil.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(list.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(str.error.frames.getCount() == 1);
    CHECK(str.error.msg == String("List index must be a number, got string."));
    CHECK(boolean.error.msg == String("List index must be a number, got bool."));
    CHECK(nil.error.msg == String("List index must be a number, got nil."));
    CHECK(list.error.msg == String("List index must be a number, got list."));
}

TEST_CASE("list index is not a whole number") {
    const auto get = runAndCollect("[1, 2][1.5];");
    const auto set = runAndCollect("var xs = [1, 2];\nxs[0.5] = 3;");
    const auto computed = runAndCollect("var xs = [1, 2, 3];\nxs[3 / 2];");

    REQUIRE(get.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(set.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(computed.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(get.error.frames.getCount() == 1);
    CHECK(get.error.msg == String("List index must be a whole number, got 1.5."));
    CHECK(set.error.msg == String("List index must be a whole number, got 0.5."));
    CHECK(computed.error.msg == String("List index must be a whole number, got 1.5."));
}

TEST_CASE("list index out of range") {
    const auto pastEnd = runAndCollect("[1, 2, 3][5];");
    const auto atEnd = runAndCollect("[1, 2, 3][3];");
    const auto negative = runAndCollect("[1, 2, 3][-1];");
    const auto empty = runAndCollect("[][0];");

    REQUIRE(pastEnd.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(atEnd.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(negative.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(empty.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(pastEnd.error.frames.getCount() == 1);
    CHECK(pastEnd.error.msg == String("List index 5 is out of range for a list of length 3."));
    CHECK(atEnd.error.msg == String("List index 3 is out of range for a list of length 3."));
    CHECK(negative.error.msg == String("List index -1 is out of range for a list of length 3."));
    CHECK(empty.error.msg == String("List index 0 is out of range for a list of length 0."));
}

TEST_CASE("assigning past the end does not grow a list") {
    const auto r = runAndCollect("var xs = [1];\nxs[1] = 2;");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.frames[0].line == 2);
    CHECK(r.error.msg == String("List index 1 is out of range for a list of length 1."));
}

TEST_CASE("list errors are checked in order: target, type, whole, range") {
    const auto targetFirst = runAndCollect("nil[1.5];");
    const auto typeBeforeRange = runAndCollect("[][\"x\"];");
    const auto wholeBeforeRange = runAndCollect("[][-0.5];");

    REQUIRE(targetFirst.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(typeBeforeRange.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(wholeBeforeRange.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(targetFirst.error.msg == String("Only lists can be indexed, got nil."));
    CHECK(typeBeforeRange.error.msg == String("List index must be a number, got string."));
    CHECK(wholeBeforeRange.error.msg == String("List index must be a whole number, got -0.5."));
}

TEST_CASE("add a list and a non-list") {
    const auto listFirst = runAndCollect("[1] + 1;");
    const auto listSecond = runAndCollect("\"a\" + [1];");
    const auto nil = runAndCollect("[] + nil;");

    REQUIRE(listFirst.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(listSecond.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(nil.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(listFirst.error.frames.getCount() == 1);
    CHECK(listFirst.error.msg == String("Operands must be two numbers, two strings or two lists."));
    CHECK(listSecond.error.msg == String("Operands must be two numbers, two strings or two lists."));
    CHECK(nil.error.msg == String("Operands must be two numbers, two strings or two lists."));
}

TEST_CASE("lists in arithmetic and comparison") {
    const auto negate = runAndCollect("-[1];");
    const auto multiply = runAndCollect("[0] * 3;");
    const auto subtract = runAndCollect("[1] - [1];");
    const auto divide = runAndCollect("[2] / 2;");
    const auto less = runAndCollect("[1] < [2];");

    REQUIRE(negate.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(multiply.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(subtract.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(divide.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(less.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(negate.error.msg == String("Operand must be a number."));
    CHECK(multiply.error.msg == String("Operands must be numbers."));
    CHECK(subtract.error.msg == String("Operands must be numbers."));
    CHECK(divide.error.msg == String("Operands must be numbers."));
    CHECK(less.error.msg == String("Operands must be numbers."));
}

TEST_CASE("lists have no methods or fields") {
    const auto method = runAndCollect("var xs = [];\nxs.push(1);");
    const auto get = runAndCollect("[].length;");
    const auto set = runAndCollect("var xs = [];\nxs.length = 1;");

    REQUIRE(method.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(get.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(set.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(method.error.msg == String("Only instances have properties."));
    CHECK(get.error.msg == String("Only instances have properties."));
    CHECK(set.error.msg == String("Only instances have fields."));
}

TEST_CASE("len of a non-list") {
    const auto str = runAndCollect("len(\"abc\");");
    const auto nil = runAndCollect("len(nil);");

    REQUIRE(str.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(nil.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(str.error.frames.getCount() == 1);
    CHECK(str.error.msg == String("len() expects a list, got string."));
    CHECK(nil.error.msg == String("len() expects a list, got nil."));
}

TEST_CASE("push to a non-list") {
    const auto r = runAndCollect("push(\"abc\", 1);");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("push() expects a list, got string."));
}

TEST_CASE("pop from a non-list") {
    const auto r = runAndCollect("pop(1);");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(r.error.frames.getCount() == 1);
    CHECK(r.error.msg == String("pop() expects a list, got number."));
}

TEST_CASE("pop from an empty list") {
    const auto literal = runAndCollect("pop([]);");
    const auto emptied = runAndCollect("var xs = [1];\npop(xs);\npop(xs);");

    REQUIRE(literal.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(emptied.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(emptied.error.frames.getCount() == 1);
    CHECK(emptied.error.frames[0].line == 3);
    CHECK(literal.error.msg == String("Cannot pop from an empty list."));
    CHECK(emptied.error.msg == String("Cannot pop from an empty list."));
}

TEST_CASE("listEquals with a non-list") {
    const auto first = runAndCollect("listEquals(1, []);");
    const auto second = runAndCollect("listEquals([], nil);");
    const auto strings = runAndCollect("listEquals(\"a\", \"a\");");

    REQUIRE(first.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(second.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(strings.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(first.error.frames.getCount() == 1);
    CHECK(first.error.msg == String("listEquals() expects two lists."));
    CHECK(second.error.msg == String("listEquals() expects two lists."));
    CHECK(strings.error.msg == String("listEquals() expects two lists."));
}

TEST_CASE("floor of a non-number") {
    const auto str = runAndCollect("floor(\"1.5\");");
    const auto list = runAndCollect("floor([1.5]);");

    REQUIRE(str.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(list.code == InterpretResultCode::RUNTIME_ERROR);
    REQUIRE(str.error.frames.getCount() == 1);
    CHECK(str.error.msg == String("floor() expects a number, got string."));
    CHECK(list.error.msg == String("floor() expects a number, got list."));
}

TEST_CASE("sort comparator returns a non-bool") {
    const auto r = runAndCollect(R"(fun before(a, b) { return a - b; }
sort([2, 1], before);)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(r.error.msg == String("sort() comparator must return true or false."));
}

TEST_CASE("list error inside a function reports the call stack") {
    const auto r = runAndCollect(R"(fun second(xs) {
    return xs[1];
}
second([1]);)");

    REQUIRE(r.code == InterpretResultCode::RUNTIME_ERROR);
    CHECK(r.error.msg == String("List index 1 is out of range for a list of length 1."));
    REQUIRE(r.error.frames.getCount() == 2);
    CHECK(r.error.frames[0].functionName == String("second"));
    CHECK(r.error.frames[0].line == 2);
    CHECK(r.error.frames[1].functionName == String("script"));
    CHECK(r.error.frames[1].line == 4);
}
