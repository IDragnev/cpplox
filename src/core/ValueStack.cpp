#include "cpplox/core/ValueStack.hpp"

namespace cpplox {
    ValueStack::ValueStack() {
        stack.reserve(256);
    }

    Value ValueStack::pop() {
        Value v = std::move(stack.back());
        stack.pop_back();
        return v;
    }

    void ValueStack::clear() {
        stack.clear();
    }
} // namespace cpplox