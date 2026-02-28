#include "doctest/doctest.h"
#include "cpplox/core/Vector.hpp"

using cpplox::Vector;

Vector<int> makeVec(std::initializer_list<int> init);

TEST_CASE("Default constructed Vector is empty") {
    Vector<int> v;

    CHECK(v.isEmpty());
    CHECK(v.getCount() == 0);
    CHECK(v.getSize() == 0);
}

TEST_CASE("Vector<int>(n) creates n zeros") {
    const std::size_t n = 5;
    Vector<int> v(n);

    CHECK(v.getCount() == n);
    CHECK(v.getSize() >= v.getCount());
    for (std::size_t i = 0; i < n; ++i) {
        CHECK(v[i] == 0);
    }
}

TEST_CASE("InsertBack adds elements and increases count") {
    const std::size_t size = 100;
    Vector<int> v;

    for (std::size_t i = 0; i < size; ++i) {
        v.insertBack(static_cast<int>(i));
    }

    CHECK(v.getCount() == size);
    CHECK(v.getSize() >= size);
    for (std::size_t i = 0; i < size; ++i) {
        CHECK(v[i] == i);
    }
}

TEST_CASE("insertAt(n, x) on empty vector appends at back") {
    Vector<int> v;

    v.insertAt(5, 42);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 1);
    CHECK(v[0] == 42);
}

TEST_CASE("insertAt(0, x) on non-empty vector inserts at front") {
    Vector<int> v;

    v.insertBack(1);
    v.insertBack(2);
    v.insertAt(0, 99);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 3);
    CHECK(v[0] == 99);
    CHECK(v[1] == 1);
    CHECK(v[2] == 2);
}

TEST_CASE("insertAt past the end appends at back") {
    Vector<int> v;
    v.insertBack(10);
    v.insertBack(20);

    std::size_t n = v.getCount();
    v.insertAt(n + 5, 99);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 3);
    CHECK(v[n] == 99);
}

TEST_CASE("insertAt at end appends element") {
    Vector<int> v;
    v.insertBack(1);
    v.insertBack(2);

    std::size_t n = v.getCount();
    v.insertAt(n, 3);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 3);
    CHECK(v[2] == 3);
}

TEST_CASE("insertAt in the middle") {
    Vector<int> v;
    v.insertBack(1);
    v.insertBack(3);
    v.insertBack(4);

    v.insertAt(1, 2);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 4);
    CHECK(v[0] == 1);
    CHECK(v[1] == 2);
    CHECK(v[2] == 3);
    CHECK(v[3] == 4);
}

TEST_CASE("removeAt(0) removes front element") {
    Vector<int> v;
    v.insertBack(1);
    v.insertBack(2);
    v.insertBack(3);

    v.removeAt(0);

    CHECK(v.getCount() == 2);
    CHECK(v.getSize() >= v.getCount());
    CHECK(v[0] == 2);
    CHECK(v[1] == 3);
}

TEST_CASE("removeAt(n/2) removes middle element") {
    Vector<int> v;
    v.insertBack(10);
    v.insertBack(20);
    v.insertBack(30);
    v.insertBack(40);

    auto m = v.getCount() / 2;
    v.removeAt(m);

    CHECK(v.getCount() == 3);
    CHECK(v.getSize() >= v.getCount());
    CHECK(v[0] == 10);
    CHECK(v[1] == 20);
    CHECK(v[2] == 40);
}

TEST_CASE("removeAt(n-1) removes last element") {
    Vector<int> v;
    v.insertBack(5);
    v.insertBack(6);
    v.insertBack(7);

    v.removeAt(v.getCount() - 1);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 2);
    CHECK(v[0] == 5);
    CHECK(v[1] == 6);
}

TEST_CASE("removeAt past the end is ignored") {
    Vector<int> v;
    v.insertBack(1);
    v.insertBack(2);

    std::size_t n = v.getCount();
    v.removeAt(n + 5);

    CHECK(v.getSize() >= v.getCount());
    CHECK(v.getCount() == 2);
    CHECK(v[0] == 1);
    CHECK(v[1] == 2);
}

TEST_CASE("Clear empties the vector") {
    Vector<int> v;

    v.insertBack(42);
    v.insertBack(99);
    v.clear();

    CHECK(v.isEmpty());
    CHECK(v.getCount() == 0);
    CHECK(v.getSize() == 0);
}

TEST_CASE("Copy constructor from empty vector") {
    Vector<int> a;
    Vector<int> b(a);

    CHECK(b.isEmpty());
}

TEST_CASE("Copy constructor from non-empty vector") {
    Vector<int> a;
    a.insertBack(1);
    a.insertBack(2);

    Vector<int> b(a);

    CHECK(b.getCount() == 2);
    CHECK(b[0] == 1);
    CHECK(b[1] == 2);
}

TEST_CASE("Move constructor from empty vector") {
    Vector<int> a;
    Vector<int> b(std::move(a));

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move constructor from non-empty vector") {
    Vector<int> a;
    a.insertBack(10);
    a.insertBack(20);

    Vector<int> b(std::move(a));

    CHECK(b.getCount() == 2);
    CHECK(b[0] == 10);
    CHECK(b[1] == 20);
    CHECK(a.isEmpty());
}

TEST_CASE("Copy assignment - empty to empty") {
    Vector<int> a;
    Vector<int> b;
    b = a;

    CHECK(b.isEmpty());
}

TEST_CASE("Copy assignment - empty to non-empty") {
    Vector<int> a;
    Vector<int> b;
    b.insertBack(1);
    b.insertBack(2);

    b = a;

    CHECK(b.isEmpty());
}

TEST_CASE("Copy assignment - non-empty to empty") {
    Vector<int> a;
    a.insertBack(3);
    a.insertBack(4);

    Vector<int> b;
    b = a;

    CHECK(b.getCount() == 2);
    CHECK(b[0] == 3);
    CHECK(b[1] == 4);
}

TEST_CASE("Copy assignment - non-empty to non-empty") {
    Vector<int> a;
    a.insertBack(5);
    a.insertBack(6);

    Vector<int> b;
    b.insertBack(99);

    b = a;
    CHECK(b.getCount() == 2);
    CHECK(b[0] == 5);
    CHECK(b[1] == 6);
}

TEST_CASE("Move assignment - empty to empty") {
    Vector<int> a;
    Vector<int> b;

    b = std::move(a);

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move assignment - empty to non-empty") {
    Vector<int> a;
    Vector<int> b;
    b.insertBack(1);

    b = std::move(a);

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move assignment - non-empty to empty") {
    Vector<int> a;
    a.insertBack(7);
    a.insertBack(8);

    Vector<int> b;
    b = std::move(a);

    CHECK(b.getCount() == 2);
    CHECK(b[0] == 7);
    CHECK(b[1] == 8);
    CHECK(a.isEmpty());
}

TEST_CASE("Move assignment - non-empty to non-empty") {
    Vector<int> a;
    a.insertBack(11);
    a.insertBack(12);

    Vector<int> b;
    b.insertBack(99);

    b = std::move(a);

    CHECK(b.getCount() == 2);
    CHECK(b[0] == 11);
    CHECK(b[1] == 12);
    CHECK(a.isEmpty());
}

TEST_CASE("Empty vectors are equal") {
    Vector<int> a;
    Vector<int> b;
    CHECK(a == b);
    CHECK_FALSE(a != b);
}

TEST_CASE("Vectors with same elements are equal") {
    Vector<int> a;
    Vector<int> b;
    a.insertBack(1);
    a.insertBack(2);
    b.insertBack(1);
    b.insertBack(2);

    CHECK(a == b);
    CHECK_FALSE(a != b);
}

TEST_CASE("Vectors with different sizes are not equal") {
    Vector<int> a;
    Vector<int> b;
    a.insertBack(1);
    a.insertBack(2);
    b.insertBack(1);

    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_CASE("Vectors with same size but different elements are not equal") {
    Vector<int> a;
    Vector<int> b;
    a.insertBack(1);
    a.insertBack(2);
    b.insertBack(1);
    b.insertBack(3);

    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_CASE("Vectors with same elements in different order are not equal") {
    Vector<int> a;
    Vector<int> b;
    a.insertBack(1);
    a.insertBack(2);
    b.insertBack(2);
    b.insertBack(1);

    CHECK_FALSE(a == b);
    CHECK(a != b);
}

TEST_SUITE("removeLastN") {
    TEST_CASE("n = 0 on empty vector") {
        Vector<int> v;

        v.removeLastN(0);

        CHECK(v.getCount() == 0);
    }

    TEST_CASE("n = 0 on non-empty vector") {
        auto v = makeVec({1, 2, 3});

        v.removeLastN(0);

        CHECK(v.getCount() == 3);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
    }

    TEST_CASE("n > getCount() removes everything") {
        auto v = makeVec({10, 20, 30});

        v.removeLastN(v.getCount() + 10);

        CHECK(v.getCount() == 0);
    }

    TEST_CASE("n < getCount() removes only the last n elements") {
        auto v = makeVec({5, 6, 7, 8, 9});

        v.removeLastN(2);

        CHECK(v.getCount() == 3);
        CHECK(v[0] == 5);
        CHECK(v[1] == 6);
        CHECK(v[2] == 7);
    }

    TEST_CASE("n == getCount() removes all elements") {
        auto v = makeVec({1, 2, 3});

        v.removeLastN(v.getCount());

        CHECK(v.getCount() == 0);
    }
}