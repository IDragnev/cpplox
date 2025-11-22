#include "doctest/doctest.h"
#include "cpplox/core/String.hpp"

#include <cstring>

using cpplox::String;

bool areEqual(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}

TEST_CASE("String() is empty") {
    String s;

    CHECK(s.size() == 0);
    CHECK(areEqual(s.c_str(), ""));
}

TEST_CASE("String(nullptr) is empty") {
    String s{nullptr};

    CHECK(s.size() == 0);
    CHECK(areEqual(s.c_str(), ""));
}

TEST_CASE("String("") is empty") {
    String s{""};

    CHECK(s.size() == 0);
    CHECK(areEqual(s.c_str(), ""));
}

TEST_CASE("String(c) is \"c\"") {
    String s{'c'};

    CHECK(s.size() == 1);
    CHECK(areEqual(s.c_str(), "c"));
}

TEST_CASE("String(const char*)") {
    String s{"c"};

    CHECK(s.size() == 1);
    CHECK(areEqual(s.c_str(), "c"));
}

TEST_CASE("String(string_view)") {
    std::string_view sv("abc");
    sv.remove_prefix(1);
    sv.remove_suffix(1);
    String s{sv};

    CHECK(s.size() == 1);
    CHECK(areEqual(s.c_str(), "b"));
}

TEST_CASE("Copy ctor from non-empty") {
    String a("abc");
    String b(a);

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Copy ctor from empty") {
    String a;
    String b(a);

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Move ctor from non-empty") {
    String a("x");
    String b = std::move(a);

    CHECK(a.size() == 0);
    CHECK(b.size() == 1);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), "x"));
}

TEST_CASE("Move ctor from empty") {
    String a;
    String b = std::move(a);
    CHECK(a.size() == 0);
    CHECK(b.size() == 0);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), ""));
}

TEST_CASE("Move assignment non-empty to non-empty") {
    String a("1234");
    String b("def");
    b = std::move(a);

    CHECK(a.size() == 0);
    CHECK(b.size() == 4);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), "1234"));
}

TEST_CASE("Move assignment empty to non-empty") {
    String a;
    String b("def");
    b = std::move(a);

    CHECK(a.size() == 0);
    CHECK(b.size() == 0);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), ""));
}

TEST_CASE("Move assignment non-empty to empty") {
    String a("abc");
    String b;
    b = std::move(a);

    CHECK(a.size() == 0);
    CHECK(b.size() == 3);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), "abc"));
}

TEST_CASE("Move assignment empty to empty") {
    String a;
    String b;
    b = std::move(a);

    CHECK(a.size() == 0);
    CHECK(b.size() == 0);
    CHECK(areEqual(a.c_str(), ""));
    CHECK(areEqual(b.c_str(), ""));
}

TEST_CASE("Copy assignment non-empty to non-empty") {
    String a("abcd");
    String b("def");
    b = a;

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Copy assignment empty to non-empty") {
    String a;
    String b("def");
    b = a;

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Copy assignment non-empty to empty") {
    String a("abc");
    String b;
    b = a;

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Copy assignment empty to empty") {
    String a;
    String b;
    b = a;

    CHECK(a.size() == b.size());
    CHECK(areEqual(a.c_str(), b.c_str()));
}

TEST_CASE("Append") {
    String str;

    str += "1";
    CHECK(str.size() == 1);
    CHECK(areEqual(str.c_str(), "1"));

    str += "23";
    CHECK(str.size() == 3);
    CHECK(areEqual(str.c_str(), "123"));

    str += "";
    CHECK(str.size() == 3);
    CHECK(areEqual(str.c_str(), "123"));
}