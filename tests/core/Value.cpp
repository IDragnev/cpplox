#include "doctest/doctest.h"
#include "cpplox/core/Value.hpp"

using cpplox::Value;

TEST_CASE("Value() is NIL") {
    Value v;
 
    CHECK(v.isNil());
}

TEST_CASE("Copy ctor for String") {
    Value a(std::string_view("abc"));
    Value b(a);

    REQUIRE(b.isString());
    CHECK(a.asString() == b.asString());
}

TEST_CASE("Copy assignment - String in NIL") {
    Value a;
    Value b(std::string_view("abc"));

    a = b;

    REQUIRE(a.isString());
    CHECK(a.asString() == b.asString());
}

TEST_CASE("Copy assignment - Boolean in String") {
    Value a(true);
    Value b(std::string_view("abc"));

    b = a;

    REQUIRE(b.isBoolean());
    CHECK(b.asBoolean());
}

TEST_CASE("Move ctor for String") {
    Value a(std::string_view("a"));
    Value b(std::move(a));

    REQUIRE(a.isNil());
    REQUIRE(b.isString());
    CHECK(b.asString() == "a");
}

TEST_CASE("Move assignment - String in Boolean") {
    Value a(false);
    Value b(std::string_view("a"));

    a = std::move(b);

    REQUIRE(b.isNil());
    REQUIRE(a.isString());
    CHECK(a.asString() == "a");
}

TEST_CASE("Move assignment - Boolean in String") {
    Value a(true);
    Value b(std::string_view("abc"));

    b = std::move(a);

    REQUIRE(a.isNil());
    REQUIRE(b.isBoolean());
    CHECK(b.asBoolean());
}
