#pragma once

namespace cpplox {
    using Value = double;

    template <typename F>
    concept ValueBinaryOp =
        requires(F f, const Value& a, const Value& b, Value c) { c = f(a, b); };

    void printValue(const Value& v);
}