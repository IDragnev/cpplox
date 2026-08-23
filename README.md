# cpplox
[![CI](https://github.com/IDragnev/cpplox/actions/workflows/ci.yml/badge.svg)](https://github.com/IDragnev/cpplox/actions)  

An interpreter for the Lox programming language - a dynamically typed language with C-like syntax, from Robert Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/). **cpplox** implements the book's bytecode interpreter and extends the language it defines, so the sections below describe **cpplox**'s version of Lox rather than the book's.

```
class Greeter {
    init(name) { this.name = name; }
    greet() { print("Hello, " + this.name + "!"); }
}

fun repeat(times, action) {
    for (var i = 0; i < times; i = i + 1) {
        action();
    }
}

var start = clock();

var greeter = Greeter("world");
repeat(2, greeter.greet);
// Hello, world!
// Hello, world!

print(clock() - start); // the milliseconds the greetings took
```

Comments start with `//` and run to the end of the line; there is no block comment form. Variable declarations, `return`, `break`, `continue` and expression statements end with a semicolon, while blocks, `if`, loops and function and class declarations do not. In the examples below, a trailing comment usually shows what the line prints.

## Getting started
There is no released binary, so the interpreter has to be built first. One command configures it, builds it and runs the tests:
```
cmake --workflow --preset windows-msvc-debug
```
Presets are named after the host and compiler, so this is the one to use on Windows with MSVC. `cmake --list-presets all` shows the ones available on your machine, and [Build](#build) explains the rest of them.

The executable is written to the build tree of the preset that produced it:
```
build/windows-msvc/app/Debug/cpplox.exe
```

Run it with the path to a script, or with no argument at all:
```
cpplox [path-to-script-file]
```
- With a path, the script is loaded and executed.
- With no argument the interpreter starts in [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) mode. To exit type *:q*. A bare expression is echoed, so `1 + 1` prints `2`; adding a semicolon makes it an ordinary statement, which prints nothing.

## The language
### Types
- **bool** - values can be *true* and *false*
- **number** - all numbers are represented as double-precision floating-point numbers. They must not have a trailing dot (*1.* is not allowed as a literal, while *1.0* and *1* are ok).
- **string** - string literals are sequences of characters enclosed in double quotes, such as *"hello"*.
- **functions** - Lox has [first-class functions](https://en.wikipedia.org/wiki/First-class_function). It supports passing functions as arguments to other functions, returning them as the values from other functions, and assigning them to variables.
- **nil** - the **nil** type has a single value - *nil*. It represents the [null value](https://en.wikipedia.org/wiki/Nullable_type). It is the value of any uninitialized variable and the default return value of functions.
- **classes** - user defined types with methods and dynamic fields. Inheritance is also supported.

### Built-in functions
A few functions are built into the interpreter rather than written in Lox. They are defined as globals before a script runs, so they are always available, and the examples below use `print` freely:

| Native | Arguments | Result |
| --- | --- | --- |
| `print(...)` | any number | Writes each argument to standard output, one per line. Returns *nil*. |
| `clock()` | none | Milliseconds since the interpreter started, always a whole number. |
| `str(v)` | 1 | *v* converted to a string, the same text `print` writes. |
| `type(v)` | 1 | The name of *v*'s type, as a string. |
| `assert(cond, msg)` | 1 or 2 | *nil* if *cond* is truthy. Otherwise stops the run with a runtime error. |

[Native functions](#native-functions) covers each of them in detail.

### Variables
Variables are declared with the *var* keyword. A variable declared without a value is *nil*:
```
var a = 10;
var b;
var c = "hello";
var d = false;

print(a); // 10
print(b); // nil
```
Names must start with a letter or an underscore, can contain letters, digits, and underscores, and cannot include spaces or special characters. They must not be reserved keywords, and they are case-sensitive, so these are two different variables:
```
var name = "lower";
var Name = "upper";
print(name); // lower
print(Name); // upper
```
Assignment is an expression and is right-associative, so a value can be given to several variables at once:
```
var a = 1;
var b = 2;
a = b = 3;
print(a); // 3
print(b); // 3
```

#### Blocks and scope
A block written with `{ }` introduces a scope. A variable declared inside it shadows any variable of the same name from an enclosing scope, and stops existing when the block ends:
```
var a = "outer";
{
    var a = "inner";
    print(a); // inner
}
print(a); // outer
```
Declaring the same name twice in one block is a compile error. Globals are the exception - redeclaring a global replaces it:
```
var a = "1";
var a = "2";
print(a); // 2
```

### Operators
#### Arithmetic
`+`, `-`, `*` and `/` work on two numbers, and `-` also negates a single one:
```
print(1 + 2);   // 3
print(5 - 1.5); // 3.5
print(3 * 4);   // 12
print(7 / 2);   // 3.5
print(-3);      // -3
```
There are no compound assignment or increment operators, so a variable is updated by assigning the result back to it, as in `i = i + 1`.

`+` additionally joins two strings:
```
print("Hello, " + "world!"); // Hello, world!
```
There is no implicit conversion, so both operands have to be numbers or both have to be strings. Mixing them is a runtime error, and `str` is what turns a value into something a message can be built from:
```
print("got " + 3);      // runtime error
print("got " + str(3)); // got 3
```
The other three operators only ever accept numbers:
```
print("a" * 2); // runtime error
```

#### Comparison
`<`, `>`, `<=` and `>=` compare two numbers and produce a bool. Comparing anything else is a runtime error:
```
print(1 < 2);  // true
print(2 > 3);  // false
print(2 <= 2); // true
print(3 >= 4); // false

print("a" < "b"); // runtime error
```

#### Equality
`==` and `!=` accept any pair of values and never fail. Values of different types are never equal, no matter how similar they look:
```
print(1 == 1);     // true
print("a" == "a"); // true
print(nil == nil); // true

print(1 == "1");     // false
print(0 == false);   // false
print(nil == false); // false
```

#### Truthiness
Every value counts as either truthy or falsey wherever a condition is expected. **nil** and **false** are falsey and everything else is truthy, including *0* and the empty string. Negating a value with `!` is the shortest way to see which one it is:
```
print(!nil);   // true, so nil is falsey
print(!false); // true

print(!0);     // false, so 0 is truthy
print(!"");    // false
```
Being truthy is not the same as being equal to *true*, which is why `0 == false` above is false.

#### Logical operators
Unlike `!`, which always produces a bool, the keywords *and* and *or* evaluate to one of their operands. `or` stops at the first truthy operand and `and` at the first falsey one; when nothing stops them, the result is the last operand, whether or not it qualifies:
```
print(nil or "fallback"); // fallback
print(1 or 2);            // 1
print(nil or false);      // false, the last operand, and falsey

print(1 and nil and 2); // nil
print(1 and 2);         // 2, the last operand, and truthy
```
Both short circuit, so the right side is not evaluated when the left one already decides the answer:
```
fun boom() {
    print("evaluated");
    return true;
}

print(true or boom()); // true, and boom was never called
```

#### Precedence
From tightest to loosest binding:

1. `!` and unary `-`
2. `*` and `/`
3. `+` and `-`
4. `<`, `>`, `<=` and `>=`
5. `==` and `!=`
6. `and`
7. `or`
8. `=`

Parentheses override the grouping:
```
print(2 + 3 * 4);     // 14
print((2 + 3) * 4);   // 20
print(1 < 2 == true); // true
```

### Control flow
#### if
The else branch is optional, and any number of branches can be chained with `else if`:
```
var a = 2;

// else is optional
if (a == 2) {
    print("two");
}

if (a == 1) {
    print("one");
}
else if (a == 2) {
    print("two");
}
else {
    print("many");
}
```

#### Loops
Lox has `while` and `for` loops.

A `while` loop repeats its body for as long as the condition stays truthy:
```
var a = 10;
while (a > 0) {
    print(a);
    a = a - 1;
}
```
A `for` loop takes three clauses separated by semicolons: an initializer, a condition checked before every iteration, and an increment that runs after each one:
```
for (var i = 10; i > 0; i = i - 1) {
    print(i);
}
```
Each of the three is optional. Leaving out the condition makes the loop run until something breaks out of it:
```
for (var i = 0;; i = i + 1) {
    if (i == 3) { break; }
    print(i); // 0, then 1, then 2
}
```

#### break and continue
`break` leaves the innermost enclosing loop, and `continue` skips the rest of the body and starts the next iteration. In a `for` loop the increment still runs before that next iteration:
```
var a = 10;
while (true) {
    if (a <= 0) {
        break;
    }
    print(a);
    a = a - 1;
}

for (var i = 0; i < 5; i = i + 1) {
    if (i == 2) { continue; } // 2 is skipped, but i is still incremented
    print(i);                 // 0, 1, 3, 4
}
```
Using either one outside a loop is a compile error.

### Functions
Functions are declared with the *fun* keyword:
```
fun sum(a, b) { return a + b; }
fun empty() { }

print(sum(1, 2)); // 3
print(empty());   // nil
```
A `return` with no value, and falling off the end of a function the way `empty` does, both produce *nil*. Printing a function shows its name and how many parameters it takes:
```
print(sum); // <fun sum:2>
```
Functions can be local and can be treated as any other value:
```
fun plus1() {
    fun local(a) {
        return a + 1;
    }

    return local;
}
var f = plus1();
print(f(2)); // 3
```

#### Closures
A function keeps access to the variables of the scope it was declared in, even after that scope has exited. Each call to the enclosing function captures its own set:
```
fun makeCounter() {
    var count = 0;
    fun increment() {
        count = count + 1;
        return count;
    }
    return increment;
}

var counter = makeCounter();
print(counter()); // 1
print(counter()); // 2

var other = makeCounter();
print(other());   // 1, its own count
print(counter()); // 3, unaffected
```

### Classes
A class is a name and a list of methods, which look like functions without the `fun` keyword. Calling the class creates an instance:
```
class Greeter {
    greet() { print("hello"); }
}

var g = Greeter();
g.greet();      // hello

print(Greeter); // <class Greeter>
print(g);       // <Greeter instance>
```
Fields are not declared anywhere. An instance gains one as soon as something assigns to it, so a method can read a field that the class definition never mentions:
```
class Box {
    show() { print(this.item); }
}

var b = Box();
b.item = "hat";
b.show();      // hat
print(b.item); // hat
```
A method named `init` is the constructor, and it receives the arguments passed to the class. Inside any method, `this` is the instance the method was called on, and it reaches that instance's other fields and methods:
```
class Person {
    init(name) { this.name = name; }
    greet() { print("I am " + this.name); }
}

var jane = Person("Jane");
jane.greet(); // I am Jane
```
A class without an `init` takes no arguments and starts its instances with no fields, the way `Greeter` and `Box` above do.

Classes are values, and so are methods. A method read off an instance stays bound to that instance, even when it is stored elsewhere:
```
var p = Person; // the class from above, held in a variable
var jane = p("Jane");
var bill = p("Bill");

bill.greet();            // I am Bill
bill.greet = jane.greet; // a field holding Jane's bound method
bill.greet();            // I am Jane
```
A class inherits from another one with `<`, which gives it every method of the superclass. Declaring a method that already exists overrides it, and `super` reaches the version that was overridden:
```
class A {
    f() { print("A"); }
}

class B < A {
    f() {
        print("B");
        super.f(); // the f that B overrode
    }
}

class C < B { } // inherits B's f, including its super call

C().f();
// B
// A
```

### Native functions
Native functions are built into the interpreter rather than written in Lox, and are defined as globals before a script runs, so they are always available. Apart from that they are ordinary values - they can be printed, passed to other functions or stored in variables and fields.

Printing a native shows its arity, which is `*` when it accepts any number of arguments and a range when it accepts a few:
```
print(print);  // <native fun print:*>
print(clock);  // <native fun clock:0>
print(assert); // <native fun assert:1-2>
```
`print` writes one line per argument. Calling it with none still writes a single blank line:
```
print("hello"); // hello
print(1, true, nil);
// 1
// true
// nil
print();        // an empty line
```
`clock` is meant for measuring how long a piece of code takes, by subtracting two readings, the way the example at the top of this file does.

`str` renders a value the way `print` does, but hands it back instead of writing it. Since `+` only joins two strings or two numbers, it is what lets a value be built into a message:
```
var n = 3;
print("got " + str(n)); // got 3
print(str(nil) + str(true)); // niltrue
```
Strings come back unchanged, without the quotes a Lox literal would need. That makes `str` unable to tell a number from its own rendering, which is what `type` is for:
```
print(type(1));      // number
print(type(str(1))); // string
```
Its answers are *nil*, *bool*, *number*, *string*, *function*, *native function*, *class* and *instance*.

`assert` checks a condition and does nothing when it holds, so it reads as a statement about the code around it. When the condition is falsey the run stops with a runtime error, reporting the second argument as the message if there is one and a generic message otherwise:
```
assert(1 + 1 == 2);
assert(1 + 1 == 3); // runtime error

var n = 3;
assert(n == 2, "expected 2, got " + str(n)); // runtime error, reporting "expected 2, got 3"
```
It is what the end-to-end tests are written with: a script that checks itself and exits with a failure has no output to compare against.

Declaring a global with the same name shadows a native for the rest of the run:
```
var clock = 10;
print(clock); // 10
```
That applies to `print` as well, which then leaves you with no way to print anything. The name holds a number, and calling a number is a runtime error. It also breaks the REPL's echo of bare expressions, since that echo is itself a call to the global `print`:
```
var print = 10;
print(1); // runtime error
```

## How it works
cpplox is a bytecode interpreter rather than a tree-walking one. A single-pass compiler emits bytecode straight from the token stream, with no AST in between, and a stack-based VM executes it. Runtime objects are freed by a mark-and-sweep collector. The `-diag` presets in [Build](#build) expose all three, turning on an instruction-by-instruction trace, a GC log, and a collection on every allocation.

## Build
The supported toolchains are described in `CMakePresets.json`, which requires **CMake 3.25** or newer. [Getting started](#getting-started) has the single command that configures, builds and tests in one go; the same thing one step at a time is:
```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

Presets are named `<os>-<compiler>-<config>`:

| Configure preset | Compiler | Build and test presets |
| --- | --- | --- |
| `windows-msvc` | MSVC | `windows-msvc-<config>` |
| `windows-clangcl` | clang-cl (Visual Studio) | `windows-clangcl-<config>` |
| `linux-gcc` | GCC | `linux-gcc-<config>` |
| `linux-clang` | Clang | `linux-clang-<config>` |
| `macos-appleclang` | Apple Clang | `macos-appleclang-<config>` |

`<config>` is `debug`, `release` or `relwithdebinfo` when building, and `debug` or `release` when testing. All of them are multi-configuration builds, so a configure preset only has to be run once and the configuration is chosen when building. Each preset gets its own build tree under `build/`. The Linux and macOS presets use the *Ninja Multi-Config* generator and need `ninja` on the `PATH`.

Only the presets matching the host operating system are usable; `cmake --list-presets all` shows them.

For debugging the interpreter itself there are `windows-msvc-diag`, `linux-gcc-diag` and `macos-appleclang-diag`, which turn on the GC log, the execution trace and the GC stress test, with `<os>-<compiler>-diag-debug` for building and testing. They skip the end-to-end tests, whose expected output does not account for the extra logging.

### Building without presets
Presets are a convenience. The project itself only requires **CMake 3.14**, so it can be configured by hand:
```
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
ctest --test-dir build -C Release
```

With a single-configuration generator the build type is chosen when configuring instead:
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

The project options are declared at the top of the root `CMakeLists.txt` and are set with `-D`, for example `-DBUILD_TESTING=OFF`. Building the tests is on by default and the end-to-end tests need Python 3.
