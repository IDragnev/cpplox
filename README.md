# An interpreter for the Lox programming language.
Lox is a dynamically typed language with syntax very close to that of C.

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
// "Hello, world!"
// "Hello, world!"

print(clock() - start); // the milliseconds the greetings took
```

## Types
- **bool** - values can be *true* and *false*
- **number** - all numbers are represented as double-precision floating-point numbers. They must not have a trailing dot (*1.* is not allowed as a literal, while *1.0* and *1* are ok).
- **string** - string literals are sequences of characters enclosed in double quotes, such as *"hello"*.
- **functions** - Lox has [first-class functions](https://en.wikipedia.org/wiki/First-class_function). It supports passing functions as arguments to other functions, returning them as the values from other functions, and assigning them to variables.
- **nil** - the **nil** type has a single value - *nil*. It represents the [null value](https://en.wikipedia.org/wiki/Nullable_type). It is the value of any uninitialized variable and the default return value of functions.
- **classes** - user defined types with methods and dynamic fields. Inheritance is also supported.

## Working with values
### Variables and functions
Variables are declared with the *var* keyword. Variable names must start with a letter or an underscore, can contain letters, digits, and underscores, and cannot include spaces or special characters. Additionally, they must not be reserved keywords and are case-sensitive:
```
var a = 10;
var A = a; // A = 10;
var b; // b = nil
var c = "hello";
var d = false;
```
Functions are declared with the *fun* keyword:
```
fun sum(a, b) { return a + b; }
fun empty() { } // returns nil
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
var a = f(2); // a = 3;
```
You can inspect values by printing them:
```
var a = 10;
fun f() {}
print(a); // 10
print(f); // <fun f:0>

```

### Native functions
Native functions are built into the interpreter rather than written in Lox, and are defined as globals before a script runs, so they are always available. Apart from that they are ordinary values - they can be printed, passed to other functions or stored in variables and fields.

There are two of them. `print` writes its arguments to standard output, one per line, and returns *nil*. It accepts any number of them, which is why its arity shows as `*`; calling it with none still writes a single blank line:
```
print("hello");   // "hello"
print(1, true, nil);
// 1
// true
// nil
print();          // an empty line
print(print);     // <native fun print:*>
```
`clock` takes no arguments and returns the whole number of milliseconds since the interpreter started. It is meant for measuring how long a piece of code takes, by subtracting two readings:
```
print(clock); // <native fun clock:0>

var start = clock();
for (var i = 0; i < 1000000; i = i + 1) { }
print(clock() - start); // the milliseconds the loop took
```
Declaring a global with the same name shadows the native for the rest of the run:
```
var clock = 10;
print(clock); // 10
```
That applies to `print` as well, which then leaves you with no way to print anything - including the REPL's echo of bare expressions, which is itself a call to the global `print`:
```
var print = 10;
print(1); // runtime error: Can only call functions and classes.
```

### Truthiness
**nil** and **false** are falsey, everything else is truthy:
```
if (0) { print("true"); } // prints "true"
if (nil) { print("oh no"); } else { print("phew"); } // prints "phew"
```

### Control flow
#### if
The else branch is optional but you can only have one else branch (no else if).
```
var a = true;

// else is optional
if (a) {
    print("yes");
}

// if and else
if (a) {
    print("yes");
}
else {
    print("no");
}
```

#### Relational operators
Relational operators are used to compare two values and return true or false depending on the comparison:

- == equal to
- != not equal to
- \> greater than
- < less than
- \>= greater than or equal to
- <= less than or equal to


#### Loops
Lox has *while* and *for* loops. Their syntax is like in C.
```
var a = 10;
while (a > 0) {
    print(a);
    a = a - 1;
}

var a = 10;
while (true) {
    if (a <= 0) {
        break;
    }
    print(a);
    a = a - 1;
}

for (var a = 10; a > 0; a = a - 1) {
    print(a);
}
```

#### Logical operators
Lox uses *!* for negation and the keywords *and* and *or* for the corresponding logical operators. They short circuit:
```
print(!true); // false

var a = false or "true" or 1 or 2;
print(a); // "true";

var b = true and 1 and nil and 1;
print(b); // nil
```

#### Classes and instances
* Classes are defined by a class name and a list of methods.
* Methods have the syntax of regular functions but without the `fun` keyword.
* Fields are not listed in the class definition - they are added to instances dynamically.
* Instances are created by 'calling' a class name like a function - `MyClass(a, b)`.
* Constructors are optional - they are special methods with the name `init`. If a class has no `init` method, its instances are created with no fields (you can add them later).
* Methods can use other class methods or fields of this instance through `this`. 
* Classes can inherit from other classes to reuse functionality. The syntax is `class SubClass < SuperClass { ... }`
* In order to call superclass methods, you should use `super`.

Example:
```
class Empty {
    print_field() { print(this.later); }
}
var e = Empty();
e.later = "hello";
e.print_field(); // "hello"
print(e.later); // "hello"

class Person {
    init(name) { this.name = name; }
    sayName() { print(this.name); }
}

var jane = Person("Jane");
jane.sayName(); // "Jane"

// classes can be stored in variables
var p = Person;

var bill = p("Bill");
bill.sayName = jane.sayName;
bill.sayName(); // "Jane" again - functions are first-class and methods bind their instance
```

Inheritance:
```
class A {
    f() { print("A"); }
}

class B < A {
    // overrides the inherited method 'f'
    f() {
        print("B");
        super.f();
    }
}

class C < B { }

C().f();
// "B"
// "A"
```

## Build
The supported toolchains are described in `CMakePresets.json`, which requires **CMake 3.25** or newer.

Configure, build and run the tests in a single command:
```
cmake --workflow --preset windows-msvc-debug
```

Or one step at a time:
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

## Usage
```
cpplox [path-to-script-file]
```

You can use the interpreter in [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) mode or by running a script file.  
- Running the interpreter with no argument loads it in REPL mode. To exit the REPL type *:q*. A bare expression is echoed, so `1 + 1` prints `2`; adding a semicolon makes it an ordinary statement, which prints nothing.
- Running the interpreter with a path to a script loads the script and tries to execute it.

