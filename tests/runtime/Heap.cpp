#include "doctest/doctest.h"
#include "cpplox/runtime/Heap.hpp"
#include "cpplox/runtime/Memory.hpp"

using namespace cpplox;

namespace {
    constexpr std::uint64_t THRESHOLD = 1024;

    struct MockObject : Object {
        static inline int alive = 0;

        MockObject() : Object(ObjectType::FUNCTION) {
            ++alive;
        }
        ~MockObject() override { --alive; }

        void trace(gc::Visitor& v) override { v.visit(child); }

        Object* child = nullptr;
        std::size_t extra = 0;

    protected:
        std::size_t computeAllocSize() const override {
            return sizeof(MockObject) + extra;
        }
    };
} // namespace

TEST_SUITE("Heap") {
    TEST_CASE("make accounts the size of the new object") {
        Heap heap(THRESHOLD);
        MockObject* obj = heap.make<MockObject>();

        REQUIRE(obj != nullptr);
        CHECK(obj->allocSize() == sizeof(MockObject));
        CHECK(heap.allocated() == sizeof(MockObject));
    }

    TEST_CASE("the destructor frees every object") {
        {
            Heap heap(THRESHOLD);
            heap.make<MockObject>();
            heap.make<MockObject>();
            CHECK(MockObject::alive == 2);
        }

        CHECK(MockObject::alive == 0);
    }

    TEST_CASE("sweeping frees untraced objects and resets survivors") {
        Heap heap(THRESHOLD);
        MockObject* kept = heap.make<MockObject>();
        heap.make<MockObject>();
        heap.make<MockObject>();

        heap.traceRoot(kept);
        heap.sweep();

        CHECK(MockObject::alive == 1);
        CHECK(heap.allocated() == sizeof(MockObject));

        heap.sweep();

        CHECK(MockObject::alive == 0);
        CHECK(heap.allocated() == 0);
    }

    TEST_CASE("tracing a root reaches the objects it references") {
        Heap heap(THRESHOLD);
        MockObject* root = heap.make<MockObject>();
        MockObject* child = heap.make<MockObject>();
        root->child = child;
        heap.make<MockObject>();

        heap.traceRoot(root);
        heap.sweep();

        CHECK(MockObject::alive == 2);
    }

    TEST_CASE("resized keeps the object and the heap in sync") {
        Heap heap(THRESHOLD);
        MockObject* obj = heap.make<MockObject>();

        obj->extra = 64;
        heap.resized(obj);
        CHECK(obj->allocSize() == sizeof(MockObject) + 64);
        CHECK(heap.allocated() == sizeof(MockObject) + 64);

        obj->extra = 16;
        heap.resized(obj);
        CHECK(obj->allocSize() == sizeof(MockObject) + 16);
        CHECK(heap.allocated() == sizeof(MockObject) + 16);

        heap.sweep();
        CHECK(heap.allocated() == 0);
    }

    TEST_CASE("adopt measures and takes ownership of every object") {
        MockObject* a = memory::makeObject<MockObject>();
        MockObject* b = memory::makeObject<MockObject>();
        a->nextObject = b;
        b->extra = 32;
        CHECK(b->allocSize() == 0);

        Heap heap(THRESHOLD);
        heap.adopt(a);

        CHECK(b->allocSize() == sizeof(MockObject) + 32);
        CHECK(heap.allocated() == 2 * sizeof(MockObject) + 32);

        heap.sweep();
        CHECK(MockObject::alive == 0);
    }

    TEST_CASE("growth past the threshold requests a collection") {
        Heap heap(sizeof(MockObject));
        MockObject* obj = heap.make<MockObject>();
        CHECK(heap.needsCollection() == false);

        obj->extra = 1;
        heap.resized(obj);
        CHECK(heap.needsCollection());
    }

    TEST_CASE("sweeping moves the threshold above the surviving bytes") {
        Heap heap(sizeof(MockObject));
        MockObject* obj = heap.make<MockObject>();
        obj->extra = 1;
        heap.resized(obj);
        REQUIRE(heap.needsCollection());

        heap.traceRoot(obj);
        heap.sweep();

        CHECK(heap.needsCollection() == false);
    }
}
