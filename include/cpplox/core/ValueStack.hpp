#pragma once

#include <vector>
#include "cpplox/core/Value.hpp"

namespace cpplox {
    class ValueStack {
    public:
        ValueStack();

        const Value& peek() const { return stack.back(); }
        Value& peek() { return stack.back(); }

        Value pop();
        void push(Value&& v) { stack.push_back(std::move(v)); }
        void push(const Value& v) { stack.push_back(v); }
        void clear();

        bool isEmpty() const { return stack.empty(); }
        std::size_t size() const { return stack.size(); }

        const Value* data() const { return stack.data(); }

    private:
        std::vector<Value> stack;
    };
} // namespace cpplox