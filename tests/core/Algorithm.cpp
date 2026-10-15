#include "doctest/doctest.h"
#include "cpplox/core/Vector.hpp"
#include "cpplox/core/Algorithm.hpp"

#include <random>
#include <vector>

using cpplox::Vector;

Vector<int> makeVec(std::initializer_list<int> init) {
    Vector<int> v;
    for (auto x : init) {
        v.insertBack(x);
    }
    return v;
}

TEST_SUITE("forEach") {
    TEST_CASE("applies function to all elements in order") {
        auto v = makeVec({1, 2, 3, 4});
        int sum = 0;

        forEach(v, [&sum](int x) { sum += x; });

        CHECK(sum == 10);
    }

    TEST_CASE("works with empty vector") {
        const Vector<int> v;
        int count = 0;

        forEach(v, [&count](int) { ++count; });

        CHECK(count == 0);
    }

    TEST_CASE("works with const vector") {
        const auto v = makeVec({5, 6, 7});
        int product = 1;

        forEach(v, [&](int x) { product *= x; });

        CHECK(product == 210);
    }
}

TEST_SUITE("contains") {
    TEST_CASE("finds an element matching the predicate") {
        const auto v = makeVec({1, 2, 3});

        CHECK(contains(v, [](int x) { return x == 2; }));
    }

    TEST_CASE("finds the first and the last element") {
        const auto v = makeVec({1, 2, 3});

        CHECK(contains(v, [](int x) { return x == 1; }));
        CHECK(contains(v, [](int x) { return x == 3; }));
    }

    TEST_CASE("no element matches") {
        const auto v = makeVec({1, 2, 3});

        CHECK_FALSE(contains(v, [](int x) { return x > 3; }));
    }

    TEST_CASE("empty vector never calls the predicate") {
        const Vector<int> v;
        int calls = 0;

        CHECK_FALSE(contains(v, [&calls](int) { ++calls; return true; }));
        CHECK(calls == 0);
    }

    TEST_CASE("stops at the first match") {
        const auto v = makeVec({1, 2, 3, 4});
        int calls = 0;

        CHECK(contains(v, [&calls](int x) { ++calls; return x == 2; }));
        CHECK(calls == 2);
    }

    TEST_CASE("predicate can match on a member") {
        struct Pair {
            int l = 0;
            int r = 0;
        };
        Vector<Pair> v;
        v.insertBack(Pair{1, 2});
        v.insertBack(Pair{3, 4});

        CHECK(contains(v, [](const Pair& p) { return p.l == 3 && p.r == 4; }));
        CHECK_FALSE(contains(v, [](const Pair& p) { return p.l == 1 && p.r == 4; }));
    }
}

TEST_SUITE("removeIf") {
    TEST_CASE("removes elements for which predicate is true") {
        auto v = makeVec({1, 2, 3, 4, 5, 6});

        removeIf(v, [](int x) { return x % 2 == 0; });

        CHECK(v.getCount() == 3);
        CHECK(v[0] == 1);
        CHECK(v[1] == 3);
        CHECK(v[2] == 5);
    }

    TEST_CASE("removeIf fuzz test for stability and correctness") {
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> distValue(0, 50);
        std::uniform_int_distribution<int> distSize(0, 50);

        for (int iter = 0; iter < 500; ++iter) {
            const int size = distSize(rng);

            Vector<int> v;
            v.reserve(size);
            std::vector<int> ref;
            ref.reserve(size);

            for (int i = 0; i < size; ++i) {
                int x = distValue(rng);
                v.insertBack(x);
                ref.push_back(x);
            }

            const auto pred = [y = distValue(rng)](int x) {
                return x < y;
            };

            std::vector<int> expected;
            for (int x : ref) {
                if (!pred(x)) {
                    expected.push_back(x);
                }
            }

            removeIf(v, pred);

            CHECK(v.getCount() == expected.size());
            for (std::size_t i = 0; i < expected.size(); ++i) {
                CHECK(v[i] == expected[i]);
            }
        }
    }

    TEST_CASE("all elements removed") {
        auto v = makeVec({1, 2, 3});

        removeIf(v, [](int) { return true; });

        CHECK(v.getCount() == 0);
    }

    TEST_CASE("no elements removed") {
        auto v = makeVec({4, 5, 6});

        removeIf(v, [](int) { return false; });

        CHECK(v.getCount() == 3);
        CHECK(v[0] == 4);
        CHECK(v[1] == 5);
        CHECK(v[2] == 6);
    }

    TEST_CASE("works on empty vector") {
        Vector<int> v;

        removeIf(v, [](int) { return true; });

        CHECK(v.getCount() == 0);
    }
}