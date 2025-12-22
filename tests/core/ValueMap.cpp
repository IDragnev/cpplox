#include "doctest/doctest.h"
#include "cpplox/core/ValueMap.hpp"
#include <string>

using cpplox::ValueMap;
using cpplox::Value;
using cpplox::String;

static bool isBool(const Value& v, bool expected) {
    return v.isBoolean() && v.asBoolean() == expected;
}

static bool isNumber(const Value& v, double expected) {
    return v.isNumber() && v.asNumber() == doctest::Approx(expected);
}

TEST_CASE("Default constructed map is empty") {
    ValueMap m;

    CHECK(m.isEmpty());
    CHECK_FALSE(m.contains("x"));
}

TEST_CASE("Insert adds entries and contains/find locate them") {
    ValueMap m;
    m.insert("a", Value(1.0));
    m.insert("b", Value(true));

    CHECK(m.contains("a"));
    CHECK(m.contains("b"));
    CHECK_FALSE(m.contains("c"));

    Value v;
    CHECK(m.find("a", v));
    CHECK(isNumber(v, 1.0));

    CHECK(m.find("b", v));
    CHECK(isBool(v, true));
}

TEST_CASE("Remove deletes existing keys") {
    ValueMap m;
    m.insert("a", Value(1.0));
    m.insert("b", Value(false));

    Value removed;
    bool found = m.remove("a", removed);

    CHECK(found);
    CHECK(isNumber(removed, 1.0));
    CHECK_FALSE(m.contains("a"));
    CHECK(m.contains("b"));
}

TEST_CASE("Remove on missing key leaves map unchanged") {
    ValueMap m;
    m.insert("a", Value(1.0));

    Value dummy(false);
    bool found = m.remove("zzz", dummy);

    CHECK_FALSE(found);
    CHECK(isBool(dummy, false));
    CHECK(m.contains("a"));
}

TEST_CASE("Insert overwrites existing key") {
    ValueMap m;
    m.insert("x", Value(1.0));
    m.insert("x", Value(2.0));

    Value v;
    CHECK(m.find("x", v));
    CHECK(isNumber(v, 2.0));
}

TEST_CASE("Clear empties the map") {
    ValueMap m;
    m.insert("a", Value(1.0));
    m.insert("b", Value(true));

    m.clear();

    CHECK(m.isEmpty());
    CHECK_FALSE(m.contains("a"));
    CHECK_FALSE(m.contains("b"));
}

TEST_CASE("Swap exchanges contents of two maps (contains + find)") {
    ValueMap a;
    a.insert("x", Value(10.0));

    ValueMap b;
    b.insert("y", Value(false));

    a.swap(b);

    CHECK(a.contains("y"));
    CHECK_FALSE(a.contains("x"));
    CHECK(b.contains("x"));
    CHECK_FALSE(b.contains("y"));

    Value v;
    CHECK(a.find("y", v));
    CHECK(isBool(v, false));
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 10.0));
}

TEST_CASE("Copy constructor from empty map") {
    ValueMap a;
    ValueMap b(a);

    CHECK(b.isEmpty());
}

TEST_CASE("Copy constructor from non-empty map") {
    ValueMap a;
    a.insert("x", Value(1.0));
    a.insert("flag", Value(true));

    ValueMap b(a);

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 1.0));
    CHECK(b.find("flag", v));
    CHECK(isBool(v, true));
}

TEST_CASE("Move constructor from empty map") {
    ValueMap a;
    ValueMap b(std::move(a));

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move constructor from non-empty map") {
    ValueMap a;
    a.insert("x", Value(1.0));

    ValueMap b(std::move(a));

    CHECK(a.isEmpty());
    CHECK(b.contains("x"));

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 1.0));
}

TEST_CASE("Copy assignment - empty to empty") {
    ValueMap a;
    ValueMap b;
    b = a;

    CHECK(b.isEmpty());
}

TEST_CASE("Copy assignment - empty to non-empty") {
    ValueMap a;
    ValueMap b;
    b.insert("k", Value(5.0));

    b = a;

    CHECK(b.isEmpty());
    CHECK_FALSE(b.contains("k"));
}

TEST_CASE("Copy assignment: non-empty to empty") {
    ValueMap a;
    a.insert("x", Value(1.0));

    ValueMap b;
    b = a;

    CHECK(b.contains("x"));

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 1.0));
}

TEST_CASE("Copy assignment - non-empty to non-empty") {
    ValueMap a;
    a.insert("x", Value(1.0));
    a.insert("y", Value(2.0));

    ValueMap b;
    b.insert("z", Value(99.0));

    b = a;

    CHECK(b.contains("x"));
    CHECK(b.contains("y"));
    CHECK_FALSE(b.contains("z"));

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 1.0));
    CHECK(b.find("y", v));
    CHECK(isNumber(v, 2.0));
}

TEST_CASE("Move assignment - empty to empty") {
    ValueMap a;
    ValueMap b;
    b = std::move(a);

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move assignment - empty to non-empty") {
    ValueMap a;
    ValueMap b;
    b.insert("k", Value(5.0));

    b = std::move(a);

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
    CHECK_FALSE(b.contains("k"));
}

TEST_CASE("Move assignment - non-empty to empty") {
    ValueMap a;
    a.insert("x", Value(7.0));

    ValueMap b;
    b = std::move(a);

    CHECK(b.contains("x"));
    CHECK(a.isEmpty());

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 7.0));
}

TEST_CASE("Move assignment - non-empty to non-empty") {
    ValueMap a;
    a.insert("x", Value(11.0));
    a.insert("y", Value(false));

    ValueMap b;
    b.insert("z", Value(99.0));

    b = std::move(a);

    CHECK(b.contains("x"));
    CHECK(b.contains("y"));
    CHECK_FALSE(b.contains("z"));
    CHECK(a.isEmpty());

    Value v;
    CHECK(b.find("x", v));
    CHECK(isNumber(v, 11.0));
    CHECK(b.find("y", v));
    CHECK(isBool(v, false));
}

TEST_CASE("Map can handle a big set of keys") {
    const auto toKey = [](std::size_t i) -> String {
        return String(std::to_string(i).c_str());
    };

    ValueMap m;
    for (std::size_t i = 0; i < 200; ++i) {
        m.insert(toKey(i), Value(double(i)));
    }

    for (std::size_t i = 0; i < 200; ++i) {
        Value v;
        CHECK(m.find(toKey(i), v));
        CHECK(isNumber(v, double(i)));
    }
}