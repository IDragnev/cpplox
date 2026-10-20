# cpplox
[![CI](https://github.com/IDragnev/cpplox/actions/workflows/ci.yml/badge.svg)](https://github.com/IDragnev/cpplox/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/IDragnev/cpplox)](https://github.com/IDragnev/cpplox/releases/latest)  

An interpreter for the Lox programming language - a dynamically typed language with C-like syntax, from Robert Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/). **cpplox** implements the book's bytecode interpreter and extends the language it defines, so the sections below describe **cpplox**'s version of Lox rather than the book's.

```
class Player {
    init(name, score) {
        this.name = name;
        this.score = score;
    }
}

fun byScore(a, b) { return a.score > b.score; }

var players = [Player("Ann", 72), Player("Bob", 95)];
push(players, Player("Cid", 88));
sort(players, byScore);

var names = [];
for (var i = 0; i < len(players); i = i + 1) {
    push(names, players[i].name);
}
print(names); // ["Bob", "Cid", "Ann"]

var best = players[0];
print(best.name + " leads with " + str(best.score)); // "Bob leads with 95"
```

Comments start with `//` and run to the end of the line; there is no block comment form. Variable declarations, `return`, `break`, `continue` and expression statements end with a semicolon, while blocks, `if`, loops and function and class declarations do not. In the examples below, a trailing comment usually shows what the line prints. `print` writes strings in double quotes, so they appear quoted in those comments too.

## Getting started
Prebuilt binaries for Windows, Linux and macOS are on the [releases page](https://github.com/IDragnev/cpplox/releases/latest). Each archive unpacks into a folder holding the interpreter and a short `INSTALL.txt` covering that platform's requirements; there is nothing to install. To build it from source instead, see [Build](#build).

Run it with the path to a script, or with no argument at all:
```
cpplox [path-to-script-file]
cpplox --version
cpplox --help
```
- With a path, the script is loaded and executed.
- With no argument the interpreter starts in [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) mode. To exit type *:q*. A bare expression is echoed the way `print` writes it, so `1 + 1` prints `2` and `"hi"` prints `"hi"`; adding a semicolon makes it an ordinary statement, which prints nothing. End a line with `\` to continue input on the next line.

A successful run exits with 0. Anything else follows the [sysexits](https://man.freebsd.org/cgi/man.cgi?query=sysexits) conventions: *64* for a usage error, *65* for a compile error, *70* for a runtime error, and *74* for a script file that cannot be read.

## The language
### Types
- **bool** - values can be *true* and *false*
- **number** - all numbers are represented as double-precision floating-point numbers. A literal needs a digit on either side of its dot, so *1.* and *.5* are both errors while *1.0*, *0.5* and *1* are ok. A magnitude may be spelled out in full or written with an exponent, as in *1e5*, *2.5E-3* or *1e+16*, and a literal too large to represent is a compile error rather than an infinity.
- **string** - string literals are sequences of characters enclosed in double quotes, such as *"hello"*. There are no escape sequences, so a backslash is an ordinary character and a literal ends at the next quote. A literal may run over several lines, and the newlines it spans are part of it.
- **functions** - Lox has [first-class functions](https://en.wikipedia.org/wiki/First-class_function). It supports passing functions as arguments to other functions, returning them as the values from other functions, and assigning them to variables.
- **nil** - the **nil** type has a single value - *nil*. It represents the [null value](https://en.wikipedia.org/wiki/Nullable_type). It is the value of any uninitialized variable and the default return value of functions.
- **classes** - user defined types with methods and dynamic fields. Inheritance is also supported.
- **list** - an ordered, growable sequence of values of any type, written with brackets, as in *[1, "two", nil]*. Lists are held by reference, like instances. See [Lists](#lists).

### Built-in functions
A few functions are built into the interpreter rather than written in Lox. They are defined as globals before a script runs, so they are always available, and the examples below use `print` freely:

| Native | Arguments | Result |
| --- | --- | --- |
| `print(...)` | any number | Writes each argument to standard output, one per line, with strings in quotes. Returns *nil*. |
| `clock()` | none | Milliseconds since the interpreter started, always a whole number. |
| `str(v)` | 1 | *v* converted to a string, the same text `print` writes, except that a string comes back unchanged. |
| `type(v)` | 1 | The name of *v*'s type, as a string. |
| `assert(cond, msg)` | 1 or 2 | *nil* if *cond* is truthy. Otherwise stops the run with a runtime error. |
| `floor(n)` | 1 | The largest whole number not greater than *n*. |
| `len(xs)` | 1 | The number of elements in the list *xs*. |
| `push(xs, v)` | 2 | Appends *v* to *xs*. Returns *nil*. |
| `pop(xs)` | 1 | Removes and returns the last element of *xs*. |
| `makeList(n, v)` | 2 | A new list of *n* elements, each *v*. |
| `listEquals(a, b)` | 2 | Whether two lists have equal contents. |

[Native functions](#native-functions) covers each of them in detail.

A few more functions are written in Lox itself, in a [prelude](#prelude) that is loaded before every script. They all work on lists:

| Function | Result |
| --- | --- |
| `contains(xs, v)` | Whether any element of *xs* is `==` to *v*. |
| `insertAt(xs, i, v)` | Inserts *v* at index *i*, shifting later elements right. Returns *nil*. |
| `removeAt(xs, i)` | Removes the element at index *i*, shifting later elements left. Returns whether it removed one. |
| `removeValue(xs, v)` | Removes the first element `==` to *v*. Returns whether it removed one. |
| `copy(xs)` | A new list with the same elements. |
| `slice(xs, low, high)` | A new list with the elements from index *low* up to but not including *high*. |
| `sort(xs, before)` | Sorts *xs* in place, using the function *before* to order elements. Returns *nil*. |

[List functions](#list-functions) covers them in detail.

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
print(name); // "lower"
print(Name); // "upper"
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
    print(a); // "inner"
}
print(a); // "outer"
```
Declaring the same name twice in one block is a compile error. Globals are the exception - redeclaring a global replaces it:
```
var a = "1";
var a = "2";
print(a); // "2"
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

`+` additionally joins two strings, or two lists into a new list:
```
print("Hello, " + "world!"); // "Hello, world!"
print([1, 2] + [3]);         // [1, 2, 3]
```
There is no implicit conversion, so both operands have to be numbers, both strings or both lists. Mixing them is a runtime error, and `str` is what turns a value into something a message can be built from:
```
print("got " + 3);      // runtime error
print("got " + str(3)); // "got 3"
```
The other three operators only ever accept numbers:
```
print("a" * 2); // runtime error
```
Dividing by zero is a runtime error rather than an infinity:
```
print(1 / 0); // runtime error
print(0 / 5); // 0, since only the divisor is rejected
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

Strings compare by their characters, but lists and instances compare by identity: two of them are equal only if they are the same object. [Lists](#lists) shows how to compare the contents of two lists.

#### Truthiness
Every value counts as either truthy or falsey wherever a condition is expected. **nil** and **false** are falsey and everything else is truthy, including *0*, the empty string and the empty list. Negating a value with `!` is the shortest way to see which one it is:
```
print(!nil);   // true, so nil is falsey
print(!false); // true

print(!0);     // false, so 0 is truthy
print(!"");    // false
print(![]);    // false
```
Being truthy is not the same as being equal to *true*, which is why `0 == false` above is false.

#### Logical operators
Unlike `!`, which always produces a bool, the keywords *and* and *or* evaluate to one of their operands. `or` stops at the first truthy operand and `and` at the first falsey one; when nothing stops them, the result is the last operand, whether or not it qualifies:
```
print(nil or "fallback"); // "fallback"
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

1. calls `()`, property access `.` and subscripts `[]`
2. `!` and unary `-`
3. `*` and `/`
4. `+` and `-`
5. `<`, `>`, `<=` and `>=`
6. `==` and `!=`
7. `and`
8. `or`
9. `=`

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
g.greet();      // "hello"

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
b.show();      // "hat"
print(b.item); // "hat"
```
A method named `init` is the constructor, and it receives the arguments passed to the class. Inside any method, `this` is the instance the method was called on, and it reaches that instance's other fields and methods:
```
class Person {
    init(name) { this.name = name; }
    greet() { print("I am " + this.name); }
}

var jane = Person("Jane");
jane.greet(); // "I am Jane"
```
A class without an `init` takes no arguments and starts its instances with no fields, the way `Greeter` and `Box` above do.

Classes are values, and so are methods. A method read off an instance stays bound to that instance, even when it is stored elsewhere:
```
var p = Person; // the class from above, held in a variable
var jane = p("Jane");
var bill = p("Bill");

bill.greet();            // "I am Bill"
bill.greet = jane.greet; // a field holding Jane's bound method
bill.greet();            // "I am Jane"
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
// "B"
// "A"
```

### Lists
A list is an ordered sequence of values, which can be of any type, lists included. It is written as comma-separated expressions in brackets:
```
var empty = [];
var mixed = [1, "two", nil, true, [3, 4]];
print(mixed); // [1, "two", nil, true, [3, 4]]
```
The elements are evaluated left to right, and every evaluation of a literal creates a new list. There is no trailing comma, so `[1, 2,]` is a compile error. A literal holds at most 255 elements, a limit that lists built with `push` or `makeList` do not have.

`nil` is an ordinary element, so `len([1, nil, 3])` is *3*.

#### Indexing
`xs[i]` reads an element and `xs[i] = v` replaces one. Indices start at 0:
```
var xs = ["a", "b", "c"];
print(xs[0]); // "a"
xs[1] = "B";
print(xs);    // ["a", "B", "c"]
```
An index must be a whole number in range, and each way of getting it wrong is its own runtime error. There are no negative indices, so `xs[-1]` is an error rather than the last element:
```
print(xs[3]);   // runtime error, out of range
print(xs[-1]);  // runtime error, out of range
print(xs[1.5]); // runtime error, not a whole number
print(xs["0"]); // runtime error, not a number
```
Since there is only one number type, `xs[1.0]` is the same as `xs[1]`. Division can produce a fraction, so index arithmetic goes through `floor`:
```
print(xs[floor(len(xs) / 2)]); // "B"
```
A subscript chains with calls and property access in any order, as in `grid[0][1]`, `this.items[0] = v` or `makeList(3, 0)[0]`. Only lists can be indexed, so `"abc"[0]` and indexing an instance are runtime errors.

Assigning past the end is an out-of-range error like reading there, so a list never grows by assignment. It grows and shrinks through functions such as `push` and `pop`:
```
var stack = [];
push(stack, 1);
push(stack, 2);
print(pop(stack)); // 2
print(len(stack)); // 1
```
`makeList` creates a list of a given length, whose slots can then be assigned:
```
var grid = makeList(3, nil);
grid[2] = "last";
print(grid); // [nil, nil, "last"]
```

#### References
A list is held by reference, like an instance. Assigning it to another variable, passing it to a function or storing it in a field shares the same list:
```
var a = [1, 2];
var b = a;
push(b, 3);
print(a); // [1, 2, 3]
```
Nothing else shares. Every operation that produces a list - `+`, `copy` and `slice` - creates a new one. Those copies are shallow, so an element that is itself a list or an instance is shared between the old list and the new one:
```
var inner = [1];
var outer = copy([inner]);
outer[0][0] = 99;
print(inner); // [99]
```
For the same reason, `makeList(2, [])` holds the same list in both slots, since its second argument is evaluated once.

#### Equality and truthiness
`==` compares lists by identity, as it does instances, and `listEquals` compares their contents. It compares elements that are lists recursively, and everything else with `==`:
```
var a = [1, [2, "x"]];
print(a == a);                       // true
print(a == [1, [2, "x"]]);           // false, a different list
print(listEquals(a, [1, [2, "x"]])); // true
```
Every list is truthy, the empty one included, so emptiness is tested with `len(xs) == 0`.

#### Printing
`print` writes a list as its elements in brackets, each one the way `print` would write it, so strings inside a list are quoted. `str` gives the same text. A list that contains itself is written as `[...]` where it repeats:
```
print(str(["a", 1])); // "["a", 1]"

var xs = [1];
push(xs, xs);
print(xs); // [1, [...]]
```

#### Iterating
A list is iterated with an index:
```
var xs = [10, 20, 30];
for (var i = 0; i < len(xs); i = i + 1) {
    print(xs[i]);
}
```
The condition reads `len(xs)` on every pass, so changing the length inside the loop is allowed, but rarely what the code means: removing an element skips the one after it, and pushing can keep the loop running forever. To remove elements while iterating, walk backwards:
```
for (var i = len(xs) - 1; i >= 0; i = i - 1) {
    if (xs[i] > 15) { removeAt(xs, i); }
}
print(xs); // [10]
```

#### List functions
`len`, `push`, `pop`, `makeList` and `listEquals` are [natives](#native-functions). They raise a runtime error when given something other than a list, and `pop` raises one on an empty list. `makeList(n, v)` needs a whole, non-negative *n*.

The rest are written in Lox, in the [prelude](#prelude). Each one checks its arguments before touching the list, so a failed call never leaves a list half-changed.

`contains` and `removeValue` look for an element with `==`, so for an element that is a list they look for *that* list, not one with the same contents:
```
var row = [1, 2];
var grid = [row];
print(contains(grid, row));    // true
print(contains(grid, [1, 2])); // false, a different list
```
`insertAt` and `removeAt` forgive an index that is out of range, but not one that is not a whole number. `insertAt` inserts at the front for a negative index and appends for one past the end, and `removeAt` returns *false* and leaves the list alone:
```
var xs = [1, 3];
insertAt(xs, 1, 2);
insertAt(xs, 100, 4);
print(xs);              // [1, 2, 3, 4]
print(removeAt(xs, 0)); // true
print(removeAt(xs, 9)); // false
print(xs);              // [2, 3, 4]
```
`slice` is strict instead: it needs `0 <= low <= high <= len(xs)`, and `low == high` gives an empty list.

`sort(xs, before)` calls `before(a, b)` to ask whether *a* should come before *b*, the same question `<` answers. It must return *true* or *false*. The sort is stable, so elements where neither comes before the other keep their original order. There are no anonymous functions, so the comparison is a named one:
```
fun shorter(a, b) { return len(a) < len(b); }

var rows = [[1, 2, 3], [4], [5, 6], [7]];
sort(rows, shorter);
print(rows); // [[4], [7], [5, 6], [1, 2, 3]]
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
print("hello"); // "hello"
print(1, true, nil);
// 1
// true
// nil
print();        // an empty line
```
Strings are written in double quotes, so `print(1)` and `print("1")` can be told apart, which matters most for the elements of a list. The quotes are for reading rather than for pasting back: string literals have no escape sequences, so a string holding a quote or a newline is written as it is.
A number is written with as many digits as it takes to read back as the same value, and switches to exponent notation for very large and very small ones. Both forms are literals, so output can be pasted back into a program:
```
print(1 / 3);              // 0.3333333333333333
print(10000000000000000);  // 1e+16
print(0.00001);            // 1e-05
```

`clock` is meant for measuring how long a piece of code takes, by subtracting two readings:
```
var start = clock();
// the code being measured
print(clock() - start); // the milliseconds it took
```

`str` renders a value the way `print` does, but hands it back instead of writing it. Since `+` only joins two values of the same type, it is what lets a value be built into a message:
```
var n = 3;
print("got " + str(n));      // "got 3"
print(str(nil) + str(true)); // "niltrue"
print("got " + str([1, 2])); // "got [1, 2]"
```
A string comes back unchanged, without the quotes `print` adds, so that it can be joined into a message as it is. Strings inside a list keep their quotes, as shown in [Lists](#printing). That makes `str` unable to tell a number from its own rendering, which is what `type` is for:
```
print(type(1));      // "number"
print(type(str(1))); // "string"
```
Its answers are *nil*, *bool*, *number*, *string*, *list*, *function*, *native function*, *class* and *instance*.

`floor` rounds a number down to a whole one, which is mostly needed for [list indices](#indexing):
```
print(floor(2.7));  // 2
print(floor(-1.5)); // -2
```

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

### Prelude
The [list functions](#list-functions) that are not natives - `contains`, `insertAt`, `removeAt`, `removeValue`, `copy`, `slice` and `sort` - are ordinary Lox functions. They live in a short Lox file that is built into the interpreter and run before every script and before the REPL starts. Being written in Lox, they print like any other function and `type` calls them functions:
```
print(contains);       // <fun contains:2>
print(type(contains)); // "function"
```
They are globals, so a script can shadow them the same way it shadows a native.

The prelude looks natives up by name when it runs, so shadowing a native such as `len` also breaks every prelude function that uses it.

## How it works
cpplox is a bytecode interpreter rather than a tree-walking one. A single-pass compiler emits bytecode straight from the token stream, with no AST in between, and a stack-based VM executes it. Runtime objects are freed by a mark-and-sweep collector. The `-diag` presets in [Build](#build) expose all three, turning on an instruction-by-instruction trace, a GC log, and a collection on every allocation. The [prelude](#prelude) is embedded in the binary as Lox source at build time and compiled at startup like any other script.

## Build
The supported toolchains are described in `CMakePresets.json`, which requires **CMake 3.25** or newer. One command configures, builds and runs the tests:
```
cmake --workflow --preset windows-msvc-debug
```
The same thing one step at a time is:
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

`<config>` is `debug`, `release` or `relwithdebinfo` when building, and `debug` or `release` when testing. All of them are multi-configuration builds, so a configure preset only has to be run once and the configuration is chosen when building. Each preset gets its own build tree under `build/`, so the executable lands somewhere like `build/windows-msvc/app/Debug/cpplox.exe`. The Linux and macOS presets use the *Ninja Multi-Config* generator and need `ninja` on the `PATH`.

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