#!/usr/bin/env python3
"""End-to-end test runner for cpplox.

Discovers .lox files, parses expectation comments, runs the cpplox
interpreter, and validates stdout/stderr/exit-code against those
expectations.

Expectation comment formats (placed anywhere in a .lox file):
    // expect: <value>                    -- expected stdout line (in order)
    // expect runtime error: <message>    -- expected runtime-error substring on stderr
    // expect compile error: <message>    -- expected compile-error on stderr
                                             (line number inferred from comment position)
    // nontest                            -- skip this file entirely

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
EXPECT_RUNTIME_ERR = re.compile(r"// expect runtime error: (.*)")
EXPECT_COMPILE_ERR = re.compile(r"// expect compile error: (.*)")
NONTEST = re.compile(r"// nontest")
ANSI_ESCAPE = re.compile(r"\x1b\[[0-9;]*m")

TIMEOUT_SECONDS = 5


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


def strip_ansi(text: str) -> str:
    """Remove ANSI escape sequences (colour codes) from *text*."""
    return ANSI_ESCAPE.sub("", text)


def parse_expectations(filepath: Path) -> Expectations:
    """Scan a .lox file for expectation comments."""
    exp = Expectations()

    with open(filepath, "r", encoding="utf-8") as fh:
        for lineno, line in enumerate(fh, start=1):
            if NONTEST.search(line):
                exp.skip = True
                return exp

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
    actual_stderr = strip_ansi(proc.stderr) if proc.stderr else ""

    if exp.compile_errors:
        expected_exit = EXIT_COMPILE_ERROR
    elif exp.runtime_error:
        expected_exit = EXIT_RUNTIME_ERROR
    else:
        expected_exit = EXIT_SUCCESS

    if proc.returncode != expected_exit:
        result.passed = False
        result.failures.append(
            f"Exit code: expected {expected_exit}, got {proc.returncode}"
        )
        for line in actual_stderr.strip().splitlines()[:5]:
            result.failures.append(f"  stderr: {line}")

    expected_lines = exp.stdout_lines
    if len(actual_stdout) != len(expected_lines):
        result.passed = False
        result.failures.append(
            f"Stdout line count: expected {len(expected_lines)}, "
            f"got {len(actual_stdout)}"
        )
    for i, expected in enumerate(expected_lines):
        actual = actual_stdout[i] if i < len(actual_stdout) else "<missing>"
        if i >= len(actual_stdout) or actual_stdout[i] != expected:
            result.passed = False
            result.failures.append(
                f"  Stdout line {i + 1}: expected '{expected}', got '{actual}'"
            )
    for i in range(len(expected_lines), len(actual_stdout)):
        result.passed = False
        result.failures.append(
            f"  Stdout line {i + 1}: unexpected '{actual_stdout[i]}'"
        )

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


def discover_tests(path: Path) -> list[Path]:
    """Return .lox files under *path* (sorted), or [*path*] if it is a file."""
    if path.is_file():
        return [path] if path.suffix == ".lox" else []
    return sorted(path.rglob("*.lox"))


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
        print("No .lox test files found.", file=sys.stderr)
        return 1

    passed = 0
    failed = 0
    skipped = 0
    failures: list[TestResult] = []

    for test_file in all_tests:
        exp = parse_expectations(test_file)

        if exp.skip:
            skipped += 1
            if args.verbose:
                print(f"  SKIP  {test_file}")
            continue

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
