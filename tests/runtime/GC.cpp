#include "doctest/doctest.h"
#include "cpplox/runtime/GC.hpp"

using namespace cpplox;

namespace {
    struct MockObject : Object {
        MockObject() : Object(ObjectType::FUNCTION, sizeof(MockObject)) {}
        void trace(gc::Visitor&) override {}

        bool wasFreed = false;
    };

    void mockDeleter(Object* obj) {
        static_cast<MockObject*>(obj)->wasFreed = true;
    }
} // namespace

TEST_SUITE("gc::sweep") {
    TEST_CASE("sweeping an empty list returns an empty result") {
        const auto result = gc::sweep(nullptr, mockDeleter);
        CHECK(result.head == nullptr);
        CHECK(result.freedBytes == 0);
    }

    TEST_CASE("sweeping an all-reachable list retains every object "
              "and resets their marks") {
        MockObject a, b, c;
        a.nextObject = &b;
        b.nextObject = &c;
        a.isReachable = true;
        b.isReachable = true;
        c.isReachable = true;

        const auto result = gc::sweep(&a, mockDeleter);

        CHECK(result.head == &a);
        CHECK(a.nextObject == &b);
        CHECK(b.nextObject == &c);
        CHECK(c.nextObject == nullptr);
        CHECK(result.freedBytes == 0);

        CHECK(a.isReachable == false);
        CHECK(b.isReachable == false);
        CHECK(c.isReachable == false);
    }

    TEST_CASE("sweeping an all-unreachable list frees every object "
              "and returns an empty list") {
        MockObject a, b, c;
        a.nextObject = &b;
        b.nextObject = &c;

        const auto result = gc::sweep(&a, mockDeleter);

        CHECK(result.head == nullptr);
        CHECK(result.freedBytes == 3 * sizeof(MockObject));
        CHECK(a.wasFreed == true);
        CHECK(b.wasFreed == true);
        CHECK(c.wasFreed == true);
    }

    TEST_CASE("sweeping frees an unreachable prefix "
              "and returns the surviving tail") {
        MockObject a, b, c, d;
        a.nextObject = &b;
        b.nextObject = &c;
        c.nextObject = &d;
        c.isReachable = true;
        d.isReachable = true;

        const auto result = gc::sweep(&a, mockDeleter);

        CHECK(result.head == &c);
        CHECK(c.nextObject == &d);
        CHECK(d.nextObject == nullptr);
        CHECK(result.freedBytes == 2 * sizeof(MockObject));
        CHECK(a.wasFreed == true);
        CHECK(b.wasFreed == true);
    }

    TEST_CASE("sweeping frees unreachable objects in the middle "
              "and links survivors") {
        MockObject a, b, c, d;
        a.nextObject = &b;
        b.nextObject = &c;
        c.nextObject = &d;
        a.isReachable = true;
        d.isReachable = true;

        const auto result = gc::sweep(&a, mockDeleter);

        CHECK(result.head == &a);
        CHECK(a.nextObject == &d);
        CHECK(d.nextObject == nullptr);
        CHECK(result.freedBytes == 2 * sizeof(MockObject));
        CHECK(b.wasFreed == true);
        CHECK(c.wasFreed == true);
    }

    TEST_CASE("sweeping frees an unreachable tail "
              "and terminates the surviving list") {
        MockObject a, b, c, d;
        a.nextObject = &b;
        b.nextObject = &c;
        c.nextObject = &d;
        a.isReachable = true;
        b.isReachable = true;

        const auto result = gc::sweep(&a, mockDeleter);

        CHECK(result.head == &a);
        CHECK(a.nextObject == &b);
        CHECK(b.nextObject == nullptr);
        CHECK(result.freedBytes == 2 * sizeof(MockObject));
        CHECK(c.wasFreed == true);
        CHECK(d.wasFreed == true);
    }
}

TEST_SUITE("gc::traceRoot") {
    TEST_CASE("tracing marks root and transitive children as reachable") {
        struct Parent : Object {
            Object* child = nullptr;
            Parent() : Object(ObjectType::FUNCTION, sizeof(Parent)) {}
            void trace(gc::Visitor& v) override { v.visit(child); }
        };

        Parent root;
        MockObject leaf;
        root.child = &leaf;

        gc::traceRoot(&root);

        CHECK(root.isReachable == true);
        CHECK(leaf.isReachable == true);
    }
}
