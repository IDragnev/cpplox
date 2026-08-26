#!/usr/bin/env python3
"""End-to-end test runner for cpplox.

Discovers .lox and .repl files, runs each one under the cpplox interpreter, and
checks the exit code plus any expectations the file declares in comments.

## .lox tests

Most tests check themselves. They call the assert native, print nothing, and
declare no expectations, so passing means exiting 0 with empty stdout. A failed
assertion aborts the run with exit 70 and reports its message on stderr, which
this runner surfaces as the failure. Write those tests in Lox and keep them out
of the expectation formats below.

The expectation comments are for the two things a script cannot assert about
itself: what the interpreter writes to stdout, and the errors that stop it from
running. They may appear anywhere in a file.

    // expect: <value>                    -- expected stdout line (in order)
    // expect empty line                  -- expected blank stdout line (in order)
    // expect runtime error: <message>    -- expected runtime-error substring on stderr
    // expect compile error: <message>    -- expected compile-error on stderr
                                             (line number inferred from comment position)
    // nontest                            -- skip this file entirely

## .repl tests

Test REPL-specific behavior (expression auto-printing, state persistence, error
recovery). The file format is line-based:

    Lines starting with "> " are input fed to the REPL via stdin.
    All other non-comment lines are expected stdout output (in order).
    Comments (lines starting with "//") are ignored.
    Error expectations use the same syntax as .lox files, placed on input lines:
        > undefined_var // expect runtime error: Undefined variable 'undefined_var'.

The interpreter is launched with no file argument (REPL mode). stdin is piped,
so the TTY-guarded prompt is suppressed and stdout contains only program output.

Exit-code conventions:
    0  = success
    65 = compile error
    70 = runtime error
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

EXIT_SUCCESS = 0
EXIT_COMPILE_ERROR = 65
EXIT_RUNTIME_ERROR = 70

EXPECT_OUTPUT = re.compile(r"// expect: (.*)")
# A blank expected line cannot be written as "// expect: " - the trailing space
# does not survive editors that trim it.
EXPECT_EMPTY_LINE = re.compile(r"// expect empty line")
EXPECT_RUNTIME_ERR = re.compile(r"// expect runtime error: (.*)")
EXPECT_COMPILE_ERR = re.compile(r"// expect compile error: (.*)")
NONTEST = re.compile(r"// nontest")
REPL_INPUT = re.compile(r"^> (.*)")
COMMENT_LINE = re.compile(r"^\s*//")
EXPECT_EMPTY_LINE_REPL = re.compile(r"^// expect empty line$")

TIMEOUT_SECONDS = 5

# Enough for a runtime error and the call stack under it. A deeper trace is
# truncated rather than allowed to bury the other failures of the run.
STDERR_EXCERPT_LINES = 15


@dataclass
class Expectations:
    stdout_lines: list[str] = field(default_factory=list)
    compile_errors: list[tuple[int, str]] = field(default_factory=list)
    runtime_error: str | None = None
    runtime_error_line: int | None = None
    skip: bool = False


@dataclass
class TestResult:
    path: str
    passed: bool
    failures: list[str] = field(default_factory=list)


def parse_expectations(filepath: Path) -> Expectations:
    """Scan a .lox file for expectation comments."""
    exp = Expectations()

    with open(filepath, "r", encoding="utf-8") as fh:
        for lineno, line in enumerate(fh, start=1):
            if NONTEST.search(line):
                exp.skip = True
                return exp

            if EXPECT_EMPTY_LINE.search(line):
                exp.stdout_lines.append("")

            m = EXPECT_OUTPUT.search(line)
            if m:
                exp.stdout_lines.append(m.group(1))

            m = EXPECT_RUNTIME_ERR.search(line)
            if m:
                exp.runtime_error = m.group(1)
                exp.runtime_error_line = lineno

            m = EXPECT_COMPILE_ERR.search(line)
            if m:
                exp.compile_errors.append((lineno, m.group(1)))

    return exp


@dataclass
class ReplExpectations:
    input_lines: list[str] = field(default_factory=list)
    expected_output: list[str] = field(default_factory=list)
    runtime_errors: list[str] = field(default_factory=list)
    compile_errors: list[str] = field(default_factory=list)
    skip: bool = False


def parse_repl_file(filepath: Path) -> ReplExpectations:
    """Parse a .repl file into input lines and expected output."""
    exp = ReplExpectations()

    with open(filepath, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")

            if NONTEST.search(line):
                exp.skip = True
                return exp

            m = REPL_INPUT.match(line)
            if m:
                input_content = m.group(1)

                err = EXPECT_RUNTIME_ERR.search(input_content)
                if err:
                    exp.runtime_errors.append(err.group(1))

                err = EXPECT_COMPILE_ERR.search(input_content)
                if err:
                    exp.compile_errors.append(err.group(1))

                code = EXPECT_RUNTIME_ERR.sub("", input_content)
                code = EXPECT_COMPILE_ERR.sub("", code).rstrip()
                exp.input_lines.append(code)
                continue

            if EXPECT_EMPTY_LINE_REPL.match(line):
                exp.expected_output.append("")
                continue

            if COMMENT_LINE.match(line):
                continue

            if line == "":
                continue

            exp.expected_output.append(line)

    return exp


def describe_exit(expected: int, actual: int) -> str:
    """Say what an unexpected exit code means, rather than just reporting it.

    The common case is a self-checking test that was expected to succeed, where
    the exit code is not the interesting part - the stderr under it is.
    """
    if expected == EXIT_SUCCESS:
        if actual == EXIT_RUNTIME_ERROR:
            return "Stopped on a runtime error, most likely a failed assertion:"
        if actual == EXIT_COMPILE_ERROR:
            return "Failed to compile:"
        return f"Exited with {actual}, expected success:"

    wanted = "a compile error" if expected == EXIT_COMPILE_ERROR else "a runtime error"
    if actual == EXIT_SUCCESS:
        return f"Expected {wanted}, but the script succeeded."

    return f"Expected {wanted} (exit {expected}), got exit {actual}:"


def run_test(interpreter: Path, filepath: Path, exp: Expectations) -> TestResult:
    """Run *filepath* under *interpreter* and validate against *exp*."""
    result = TestResult(path=str(filepath), passed=True)

    try:
        proc = subprocess.run(
            [str(interpreter), str(filepath)],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SECONDS,
        )
    except subprocess.TimeoutExpired:
        result.passed = False
        result.failures.append(f"Timed out after {TIMEOUT_SECONDS}s")
        return result
    except OSError as e:
        result.passed = False
        result.failures.append(f"Failed to run interpreter: {e}")
        return result

    actual_stdout = proc.stdout.splitlines() if proc.stdout else []
    actual_stderr = proc.stderr if proc.stderr else ""

    if exp.compile_errors:
        expected_exit = EXIT_COMPILE_ERROR
    elif exp.runtime_error:
        expected_exit = EXIT_RUNTIME_ERROR
    else:
        expected_exit = EXIT_SUCCESS

    if proc.returncode != expected_exit:
        result.passed = False
        result.failures.append(describe_exit(expected_exit, proc.returncode))
        excerpt = actual_stderr.strip().splitlines()
        for line in excerpt[:STDERR_EXCERPT_LINES]:
            result.failures.append(f"  {line}")
        if len(excerpt) > STDERR_EXCERPT_LINES:
            omitted = len(excerpt) - STDERR_EXCERPT_LINES
            result.failures.append(f"  ... {omitted} more stderr lines")

    # Every line is accounted for in both directions, so a test that declares no
    # expectations is also a test that the script writes nothing.
    expected_lines = exp.stdout_lines
    for i, expected in enumerate(expected_lines):
        if i >= len(actual_stdout):
            result.passed = False
            result.failures.append(f"Stdout line {i + 1}: missing '{expected}'")
        elif actual_stdout[i] != expected:
            result.passed = False
            result.failures.append(
                f"Stdout line {i + 1}: expected '{expected}', "
                f"got '{actual_stdout[i]}'"
            )
    for i in range(len(expected_lines), len(actual_stdout)):
        result.passed = False
        result.failures.append(f"Stdout line {i + 1}: unexpected '{actual_stdout[i]}'")

    for lineno, message in exp.compile_errors:
        needle = f"Compile error on line {lineno}: {message}"
        if needle not in actual_stderr:
            result.passed = False
            result.failures.append(f"Missing compile error: '{needle}'")

    if exp.runtime_error:
        needle = f"Runtime error: {exp.runtime_error}"
        if needle not in actual_stderr:
            result.passed = False
            result.failures.append(f"Missing runtime error: '{needle}'")

    return result


def run_repl_test(interpreter: Path, filepath: Path, exp: ReplExpectations) -> TestResult:
    """Run a .repl test by piping input to the interpreter in REPL mode."""
    result = TestResult(path=str(filepath), passed=True)

    stdin_text = "\n".join(exp.input_lines) + "\n"

    try:
        proc = subprocess.run(
            [str(interpreter)],
            input=stdin_text,
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SECONDS,
        )
    except subprocess.TimeoutExpired:
        result.passed = False
        result.failures.append(f"Timed out after {TIMEOUT_SECONDS}s")
        return result
    except OSError as e:
        result.passed = False
        result.failures.append(f"Failed to run interpreter: {e}")
        return result

    if proc.returncode != EXIT_SUCCESS:
        result.passed = False
        result.failures.append(
            f"REPL exited with {proc.returncode}, expected 0:"
        )
        excerpt = (proc.stderr or "").strip().splitlines()
        for line in excerpt[:STDERR_EXCERPT_LINES]:
            result.failures.append(f"  {line}")
        if len(excerpt) > STDERR_EXCERPT_LINES:
            omitted = len(excerpt) - STDERR_EXCERPT_LINES
            result.failures.append(f"  ... {omitted} more stderr lines")

    actual_stdout = proc.stdout.splitlines() if proc.stdout else []
    actual_stderr = proc.stderr or ""

    for i, expected in enumerate(exp.expected_output):
        if i >= len(actual_stdout):
            result.passed = False
            result.failures.append(f"Stdout line {i + 1}: missing '{expected}'")
        elif actual_stdout[i] != expected:
            result.passed = False
            result.failures.append(
                f"Stdout line {i + 1}: expected '{expected}', "
                f"got '{actual_stdout[i]}'"
            )
    for i in range(len(exp.expected_output), len(actual_stdout)):
        result.passed = False
        result.failures.append(f"Stdout line {i + 1}: unexpected '{actual_stdout[i]}'")

    for message in exp.runtime_errors:
        needle = f"Runtime error: {message}"
        if needle not in actual_stderr:
            result.passed = False
            result.failures.append(f"Missing runtime error: '{needle}'")

    for message in exp.compile_errors:
        needle = f"Compile error: {message}"
        if needle not in actual_stderr:
            result.passed = False
            result.failures.append(f"Missing compile error: '{needle}'")

    return result


def discover_tests(path: Path) -> list[Path]:
    """Return .lox and .repl files under *path* (sorted), or [*path*] if it is a file."""
    if path.is_file():
        return [path] if path.suffix in (".lox", ".repl") else []
    lox_files = path.rglob("*.lox")
    repl_files = path.rglob("*.repl")
    return sorted([*lox_files, *repl_files])


def main() -> int:
    parser = argparse.ArgumentParser(description="cpplox end-to-end test runner")
    parser.add_argument(
        "--interpreter", required=True, help="Path to the cpplox executable"
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=None,
        help="Test files or directories (default: tests/e2e/)",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Show each test result"
    )
    args = parser.parse_args()

    interpreter = Path(args.interpreter)
    if not interpreter.exists():
        print(f"Error: interpreter not found: {interpreter}", file=sys.stderr)
        return 1

    test_paths = [Path(p) for p in args.paths] if args.paths else [Path(__file__).parent]

    all_tests: list[Path] = []
    for p in test_paths:
        all_tests.extend(discover_tests(p))

    if not all_tests:
        print("No test files found.", file=sys.stderr)
        return 1

    passed = 0
    failed = 0
    skipped = 0
    failures: list[TestResult] = []

    for test_file in all_tests:
        if test_file.suffix == ".repl":
            exp = parse_repl_file(test_file)
        else:
            exp = parse_expectations(test_file)

        if exp.skip:
            skipped += 1
            if args.verbose:
                print(f"  SKIP  {test_file}")
            continue

        if test_file.suffix == ".repl":
            result = run_repl_test(interpreter, test_file, exp)
        else:
            result = run_test(interpreter, test_file, exp)

        if result.passed:
            passed += 1
            if args.verbose:
                print(f"  PASS  {test_file}")
        else:
            failed += 1
            failures.append(result)
            if args.verbose:
                print(f"  FAIL  {test_file}")
                for f in result.failures:
                    print(f"        {f}")

    print()
    if failures:
        print("Failures:")
        print()
        for r in failures:
            print(f"  FAIL  {r.path}")
            for f in r.failures:
                print(f"        {f}")
            print()

    total = passed + failed + skipped
    print(f"{total} tests: {passed} passed, {failed} failed, {skipped} skipped")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
