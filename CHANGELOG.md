# Changelog

Notable changes to cpplox. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and releases follow
[semantic versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Lists: an ordered, growable, mutable sequence of values of any type, held by
  reference like instances.
  - Literals such as `[1, "two", nil]`, evaluated left to right, with at most
    255 elements.
  - Subscripts `xs[i]` and `xs[i] = v`, chaining with calls and property
    access. An index must be a whole number in range; anything else is a
    runtime error.
  - `+` joins two lists into a new one, and `==` compares lists by identity.
  - `print` and `str` write lists as `[1, "two", nil]`, and a list that
    contains itself as `[...]` where it repeats.
  - `type` answers `"list"`.
- Native functions `len`, `push`, `pop`, `makeList`, `listEquals` and `floor`.
- A prelude of list functions written in Lox and loaded before every script and
  the REPL: `contains`, `insertAt`, `removeAt`, `removeValue`, `copy`, `slice`
  and a stable `sort` that takes a comparison function.

### Changed

- `print` writes strings in double quotes, and so does the REPL's echo of a
  bare expression: `print("hi")` now writes `"hi"`. `str` still returns a
  string unchanged, but quotes the strings inside a list.

### Fixed

- A source that ended right after a prefix operator or an opening bracket, such
  as `-` or `(`, crashed the interpreter instead of reporting "Expected
  expression".

## [0.1.0] - 2026-09-12

First public release.

### Language

- Dynamic types: `bool`, `number`, `string`, `nil`, functions and classes.
- Numbers are double-precision, written plainly or with an exponent (`1e5`,
  `2.5E-3`); a literal too large to represent is a compile error.
- Variables, block scope, and arithmetic, comparison, equality and logical
  operators, with `and` and `or` short-circuiting. Division by zero is a
  runtime error rather than an infinity.
- `if`/`else`, `while` and `for`, with `break` and `continue`.
- First-class functions and closures.
- Classes with fields, methods, bound methods, `this`, an `init` constructor,
  and single inheritance with `super`.
- Native functions `print`, `clock`, `str`, `type` and `assert`, defined as
  globals before a script runs.

### Running it

- Runs a script file, or starts a REPL that keeps state across lines and
  recovers from errors.
- `cpplox --version` and `cpplox --help`.

[Unreleased]: https://github.com/IDragnev/cpplox/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/IDragnev/cpplox/releases/tag/v0.1.0
