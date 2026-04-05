#pragma once

#include "cpplox/core/Value.hpp"
#include "cpplox/core/Vector.hpp"

namespace cpplox {
    class ValueStack {
    public:
        const Value& at(std::size_t i) const { return stack[i]; }
        Value& at(std::size_t i) { return stack[i]; }

        const Value& peekN(std::size_t n) const;
        Value& peekN(std::size_t n);
        const Value& peek() const { return stack.back(); }
        Value& peek() { return stack.back(); }

        Value pop();
        void popN(std::size_t n);
        void push(Value&& v) { stack.insertBack(std::move(v)); }
        void push(const Value& v) { stack.insertBack(v); }
        void clear();
        void reserve(std::size_t size);

        bool isEmpty() const { return stack.isEmpty(); }
        std::size_t size() const { return stack.getCount(); }

        const Value* data() const { return stack.data(); }

    private:
        Vector<Value> stack;
    };
} // namespace cpplox