#include "doctest/doctest.h"
#include "cpplox/runtime/ValueMap.hpp"
#include "cpplox/runtime/RuntimeStr.hpp"
#include <deque>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using cpplox::RuntimeStr;
using cpplox::String;
using cpplox::Value;
using cpplox::ValueMap;

namespace std {
    template <>
    struct hash<String> {
        std::size_t operator()(const String& s) const noexcept {
            return static_cast<std::size_t>(s.hashValue());
        }
    };
} // namespace std

namespace {
    // The map stores key pointers only, so the keys must outlive it.
    // std::deque never moves its elements when it grows.
    class Keys {
    public:
        RuntimeStr* operator()(std::string_view s) {
            return &storage.emplace_back(s);
        }

    private:
        std::deque<RuntimeStr> storage;
    };
} // namespace

static bool isBool(const Value& v, bool expected) {
    return v.isBoolean() && v.asBoolean() == expected;
}

static bool isNumber(const Value& v, double expected) {
    return v.isNumber() && v.asNumber() == doctest::Approx(expected);
}

TEST_CASE("Default constructed map is empty") {
    Keys key;
    ValueMap m;

    CHECK(m.isEmpty());
    CHECK_FALSE(m.contains(key("x")));
}

TEST_CASE("Insert adds entries and contains/find locate them") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));
    m.insert(key("b"), Value(true));

    CHECK(m.contains(key("a")));
    CHECK(m.contains(key("b")));
    CHECK_FALSE(m.contains(key("c")));

    Value v;
    CHECK(m.find(key("a"), v));
    CHECK(isNumber(v, 1.0));

    CHECK(m.find(key("b"), v));
    CHECK(isBool(v, true));
}

TEST_CASE("Keys are compared by content, not identity") {
    RuntimeStr inserted(std::string_view("name"));
    RuntimeStr lookup(std::string_view("name"));
    RuntimeStr other(std::string_view("other"));
    REQUIRE(&inserted != &lookup);

    ValueMap m;
    m.insert(&inserted, Value(1.0));

    Value v;
    CHECK(m.find(&lookup, v));
    CHECK(isNumber(v, 1.0));
    CHECK_FALSE(m.contains(&other));

    m.insert(&lookup, Value(2.0));
    CHECK(m.find(&inserted, v));
    CHECK(isNumber(v, 2.0));
}

TEST_CASE("forEach visits every key and value") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));
    m.insert(key("b"), Value(2.0));
    m.insert(key("c"), Value(3.0));

    std::unordered_map<String, double> seen;
    m.forEach([&seen](RuntimeStr* k, Value& v) {
        REQUIRE(k != nullptr);
        REQUIRE(v.isNumber());
        seen[k->str] = v.asNumber();
    });

    CHECK(seen.size() == 3);
    CHECK(seen[String("a")] == doctest::Approx(1.0));
    CHECK(seen[String("b")] == doctest::Approx(2.0));
    CHECK(seen[String("c")] == doctest::Approx(3.0));
}

TEST_CASE("nullptr keys are ignored") {
    Keys key;
    ValueMap m;
    m.insert(nullptr, Value(1.0));
    CHECK(m.isEmpty());

    m.insert(key("a"), Value(2.0));

    Value v(false);
    CHECK_FALSE(m.find(nullptr, v));
    CHECK(isBool(v, false));
    CHECK_FALSE(m.contains(nullptr));
    CHECK_FALSE(m.remove(nullptr, v));
    CHECK(isBool(v, false));
    CHECK(m.contains(key("a")));
}

TEST_CASE("Remove deletes existing keys") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));
    m.insert(key("b"), Value(false));

    Value removed;
    bool found = m.remove(key("a"), removed);

    CHECK(found);
    CHECK(isNumber(removed, 1.0));
    CHECK_FALSE(m.contains(key("a")));
    CHECK(m.contains(key("b")));
}

TEST_CASE("Remove on missing key leaves map unchanged") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));

    Value dummy(false);
    bool found = m.remove(key("zzz"), dummy);

    CHECK_FALSE(found);
    CHECK(isBool(dummy, false));
    CHECK(m.contains(key("a")));
}

TEST_CASE("Removing the only key empties the map") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));

    Value removed;
    REQUIRE(m.remove(key("a"), removed));

    CHECK(m.isEmpty());
}

TEST_CASE("Removing every key empties the map") {
    const int N = 200;
    Keys key;
    ValueMap m;
    for (int i = 0; i < N; ++i) {
        m.insert(key("k" + std::to_string(i)), Value(double(i)));
    }

    for (int i = 0; i < N; ++i) {
        CHECK_FALSE(m.isEmpty());

        Value removed;
        REQUIRE(m.remove(key("k" + std::to_string(i)), removed));
        CHECK(isNumber(removed, double(i)));

        for (int j = i + 1; j < N; ++j) {
            REQUIRE(m.contains(key("k" + std::to_string(j))));
        }
    }

    CHECK(m.isEmpty());
}

TEST_CASE("Insert overwrites existing key") {
    Keys key;
    ValueMap m;
    m.insert(key("x"), Value(1.0));
    m.insert(key("x"), Value(2.0));

    Value v;
    CHECK(m.find(key("x"), v));
    CHECK(isNumber(v, 2.0));
}

TEST_CASE("Clear empties the map") {
    Keys key;
    ValueMap m;
    m.insert(key("a"), Value(1.0));
    m.insert(key("b"), Value(true));

    m.clear();

    CHECK(m.isEmpty());
    CHECK_FALSE(m.contains(key("a")));
    CHECK_FALSE(m.contains(key("b")));
}

TEST_CASE("Swap exchanges contents of two maps") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(10.0));

    ValueMap b;
    b.insert(key("y"), Value(false));

    a.swap(b);

    CHECK(a.contains(key("y")));
    CHECK_FALSE(a.contains(key("x")));
    CHECK(b.contains(key("x")));
    CHECK_FALSE(b.contains(key("y")));

    Value v;
    CHECK(a.find(key("y"), v));
    CHECK(isBool(v, false));
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 10.0));
}

TEST_CASE("Copy constructor from empty map") {
    ValueMap a;
    ValueMap b(a);

    CHECK(b.isEmpty());
}

TEST_CASE("Copy constructor from non-empty map") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(1.0));
    a.insert(key("flag"), Value(true));

    ValueMap b(a);

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 1.0));
    CHECK(b.find(key("flag"), v));
    CHECK(isBool(v, true));
}

TEST_CASE("Move constructor from empty map") {
    ValueMap a;
    ValueMap b(std::move(a));

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
}

TEST_CASE("Move constructor from non-empty map") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(1.0));

    ValueMap b(std::move(a));

    CHECK(a.isEmpty());
    CHECK(b.contains(key("x")));

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 1.0));
}

TEST_CASE("Copy assignment - empty to empty") {
    ValueMap a;
    ValueMap b;
    b = a;

    CHECK(b.isEmpty());
}

TEST_CASE("Copy assignment - empty to non-empty") {
    Keys key;
    ValueMap a;
    ValueMap b;
    b.insert(key("k"), Value(5.0));

    b = a;

    CHECK(b.isEmpty());
    CHECK_FALSE(b.contains(key("k")));
}

TEST_CASE("Copy assignment - non-empty to empty") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(1.0));

    ValueMap b;
    b = a;

    CHECK(b.contains(key("x")));

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 1.0));
}

TEST_CASE("Copy assignment - non-empty to non-empty") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(1.0));
    a.insert(key("y"), Value(2.0));

    ValueMap b;
    b.insert(key("z"), Value(99.0));

    b = a;

    CHECK(b.contains(key("x")));
    CHECK(b.contains(key("y")));
    CHECK_FALSE(b.contains(key("z")));

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 1.0));
    CHECK(b.find(key("y"), v));
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
    Keys key;
    ValueMap a;
    ValueMap b;
    b.insert(key("k"), Value(5.0));

    b = std::move(a);

    CHECK(b.isEmpty());
    CHECK(a.isEmpty());
    CHECK_FALSE(b.contains(key("k")));
}

TEST_CASE("Move assignment - non-empty to empty") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(7.0));

    ValueMap b;
    b = std::move(a);

    CHECK(b.contains(key("x")));
    CHECK(a.isEmpty());

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 7.0));
}

TEST_CASE("Move assignment - non-empty to non-empty") {
    Keys key;
    ValueMap a;
    a.insert(key("x"), Value(11.0));
    a.insert(key("y"), Value(false));

    ValueMap b;
    b.insert(key("z"), Value(99.0));

    b = std::move(a);

    CHECK(b.contains(key("x")));
    CHECK(b.contains(key("y")));
    CHECK_FALSE(b.contains(key("z")));
    CHECK(a.isEmpty());

    Value v;
    CHECK(b.find(key("x"), v));
    CHECK(isNumber(v, 11.0));
    CHECK(b.find(key("y"), v));
    CHECK(isBool(v, false));
}

TEST_CASE("ValueMap handles many random keys correctly") {
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(0, 1000000);

    const std::size_t N = 500;
    std::vector<std::string> names;
    names.reserve(N);

    Keys key;
    ValueMap m;
    for (std::size_t i = 0; i < N; ++i) {
        names.push_back("k" + std::to_string(dist(rng)));
        m.insert(key(names.back()), Value(double(i)));
    }

    for (std::size_t i = 0; i < N; ++i) {
        Value v;
        CHECK(m.find(key(names[i]), v));
        CHECK(isNumber(v, double(i)));
    }
}

TEST_CASE("Random mixed operations under load behave correctly") {
    const auto makeBaseKey = [](int i) {
        String key("base");
        key += std::to_string(i).c_str();
        return key;
    };
    enum class Op { INSERT, REMOVE, FIND };
    const int minOp = static_cast<int>(Op::INSERT);
    const int maxOp = static_cast<int>(Op::FIND);
    class Random {
    public:
        Random() : rng(98765), keyDist(0, 1000000), opDist(minOp, maxOp) {}
        String key() {
            String key("k");
            key += std::to_string(keyDist(rng)).c_str();
            return key;
        }
        Op op() {
            return static_cast<Op>(opDist(rng));
        }
    private:
        std::mt19937 rng;
        std::uniform_int_distribution<int> keyDist;
        std::uniform_int_distribution<int> opDist;
    } random;

    Keys key;
    ValueMap map;
    std::unordered_map<String, Value> refMap;

    const int PREFILL = 300;
    for (int i = 0; i < PREFILL; ++i) {
        String name = makeBaseKey(i);
        auto val = Value{double(i)};
        refMap[name] = val;
        map.insert(key(name.view()), val);
    }

    const int OPS = 2000;
    for (int i = 0; i < OPS; ++i) {
        String name = random.key();
        switch (random.op()) {
            case Op::INSERT: {
                auto val = Value{double(i)};
                map.insert(key(name.view()), val);
                refMap[name] = val;
            } break;
            case Op::REMOVE: {
                Value removed;
                bool found = map.remove(key(name.view()), removed);

                auto it = refMap.find(name);
                if (it != refMap.end()) {
                    CHECK(found);
                    CHECK(removed == it->second);
                    refMap.erase(it);
                } else {
                    CHECK_FALSE(found);
                    CHECK(removed.isNil());
                }
            } break;
            case Op::FIND: {
                Value v;
                bool found = map.find(key(name.view()), v);

                auto it = refMap.find(name);
                if (it != refMap.end()) {
                    CHECK(found);
                    CHECK(v == it->second);
                } else {
                    CHECK_FALSE(found);
                }
            } break;
        }

        if (i % 200 == 0) {
            for (int j = 0; j < PREFILL; ++j) {
                CHECK(map.contains(key(makeBaseKey(j).view())));
            }
        }
    }

    for (const auto& [name, val] : refMap) {
        Value v;
        CHECK(map.find(key(name.view()), v));
        CHECK(v == val);
    }
}
