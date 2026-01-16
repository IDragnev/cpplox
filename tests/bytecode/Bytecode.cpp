#include "doctest/doctest.h"
#include "cpplox/bytecode/Bytecode.hpp"

TEST_CASE("fitsOneByte") {
    using cpplox::fitsOneByte;

    CHECK(fitsOneByte(0));
    CHECK(fitsOneByte(1));
    CHECK(fitsOneByte(255));
    CHECK_FALSE(fitsOneByte(256));
    CHECK_FALSE(fitsOneByte(1000));
}

TEST_CASE("fitsTwoBytes") {
    using cpplox::fitsTwoBytes;

    CHECK(fitsTwoBytes(0));
    CHECK(fitsTwoBytes(255));
    CHECK(fitsTwoBytes(256));
    CHECK(fitsTwoBytes(65535));
    CHECK_FALSE(fitsTwoBytes(65536));
    CHECK_FALSE(fitsTwoBytes(999999));
}

TEST_SUITE("Two-byte (de)serialization") {
    using cpplox::parseTwoByteInteger;
    using cpplox::serializeTwoByteInteger;

    TEST_CASE("round-trip: serialize then parse") {
        const std::size_t values[] = {0, 1, 255, 256, 1024, 65535};
        for (const std::size_t v : values) {
            std::uint8_t a = 0, b = 0;
            serializeTwoByteInteger(v, a, b);
            const std::size_t parsed = parseTwoByteInteger(a, b);
            CHECK(parsed == v);
        }
    }

    TEST_CASE("serialization produces correct bytes (little-endian)") {
        std::uint8_t a = 0, b = 0;

        serializeTwoByteInteger(0x1234, a, b);
        CHECK(a == 0x34);
        CHECK(b == 0x12);

        serializeTwoByteInteger(0x00FF, a, b);
        CHECK(a == 0xFF);
        CHECK(b == 0x00);

        serializeTwoByteInteger(0xFF00, a, b);
        CHECK(a == 0x00);
        CHECK(b == 0xFF);
    }

    TEST_CASE("parseTwoByteInteger reconstructs correctly (little-endian)") {
        CHECK(parseTwoByteInteger(0x12, 0x34) == 0x3412);
        CHECK(parseTwoByteInteger(0xFF, 0x00) == 0x00FF);
        CHECK(parseTwoByteInteger(0x00, 0xFF) == 0xFF00);
    }
}