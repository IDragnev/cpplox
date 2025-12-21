#include "cpplox/core/ValueStack.hpp"

namespace cpplox {
    ValueStack::ValueStack() {
        stack.reserve(256);
    }

    Value ValueStack::pop() {
        Value v = std::move(stack.back());
        stack.removeBack();
        return v;
    }

    const Value& ValueStack::peekN(std::size_t n) {
        const std::size_t i = stack.getCount() - 1 - n;
        return stack[i];
    }

    void ValueStack::clear() {
        stack.clear();
    }
} // namespace cpplox