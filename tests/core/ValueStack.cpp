#include "doctest/doctest.h"
#include "cpplox/core/ValueStack.hpp"

using cpplox::ValueStack;
using cpplox::Value;

TEST_CASE("Default Stack is empty") {
    ValueStack s;

    CHECK(s.isEmpty());
    CHECK(s.size() == 0);
}

TEST_CASE("Push increases size and peek returns last element") {
    ValueStack s;
    s.push(Value(false));
    s.push(Value(true));

    CHECK(s.size() == 2);
    CHECK(s.peek().asBoolean());
    CHECK_FALSE(s.isEmpty());
}

TEST_CASE("Pop returns last element and decreases size") {
    ValueStack s;
    s.push(Value(true));
    s.push(Value(false));

    Value v = s.pop();

    CHECK_FALSE(v.asBoolean());
    CHECK(s.size() == 1);
    CHECK(s.peek().asBoolean());
    CHECK_FALSE(s.isEmpty());
}

TEST_CASE("PeekN returns nth-from-top element") {
    ValueStack s;
    s.push(Value(5.0));
    s.push(Value(6.0));
    s.push(Value(7.0));

    CHECK(s.peekN(0).asNumber() == 7.0);
    CHECK(s.peekN(1).asNumber() == 6.0);
    CHECK(s.peekN(2).asNumber() == 5.0);
}

TEST_CASE("Clear empties the stack") {
    ValueStack s;
    s.push(Value());
    s.push(Value());
    s.clear();

    CHECK(s.isEmpty());
    CHECK(s.size() == 0);
}