#include "cpplox/core/ValueStack.hpp"

namespace cpplox {
    void ValueStack::reserve(std::size_t size) {
        stack.reserve(size);
    }

    Value ValueStack::pop() {
        Value v = std::move(stack.back());
        stack.removeBack();
        return v;
    }

    void ValueStack::popN(std::size_t n) {
        for (; n > 0; --n) {
            if (isEmpty()) {
                break;
            }

            stack.removeBack();
        }
    }

    const Value& ValueStack::peekN(std::size_t n) const {
        const std::size_t i = stack.getCount() - 1 - n;
        return stack[i];
    }

    Value& ValueStack::peekN(std::size_t n) {
        const std::size_t i = stack.getCount() - 1 - n;
        return stack[i];
    }

    void ValueStack::clear() {
        stack.clear();
    }
} // namespace cpplox