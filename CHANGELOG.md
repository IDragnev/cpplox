# Changelog

Notable changes to cpplox. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and releases follow
[semantic versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
