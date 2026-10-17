#include "cpplox/runtime/Heap.hpp"
#include "cpplox/runtime/GCVisitor.hpp"
#ifdef CPPLOX_DEBUG_LOG_GC
    #include "cpplox/log/Log.hpp"
#endif

namespace cpplox {
    namespace {
        class Marker : public gc::Visitor {
        public:
            void visit(Object* obj) override;
        };

        void Marker::visit(Object* obj) {
            if (obj == nullptr || obj->isReachable) {
                return;
            }
#ifdef CPPLOX_DEBUG_LOG_GC
            println("Mark object of type {} at {} as reachable",
                    static_cast<int>(obj->type()),
                    static_cast<void*>(obj));
#endif

            obj->isReachable = true;
            obj->trace(*this);
        }
    } // namespace

    Heap::~Heap() {
        while (objects != nullptr) {
            Object* next = objects->nextObject;
            memory::freeObject(objects);
            objects = next;
        }
    }

    void Heap::adopt(Object* list) {
        while (list != nullptr) {
            Object* next = list->nextObject;
            link(list);
            list = next;
        }
    }

    void Heap::resized(Object* obj) {
        const std::size_t oldSize = obj->allocSize();
        obj->updateAllocSize(SizeKey{});
        bytesAllocated = bytesAllocated - oldSize + obj->allocSize();
    }

    void Heap::traceRoot(Object* root) {
#ifdef CPPLOX_DEBUG_LOG_GC
        println("Start trace with root of type {} at {}",
                static_cast<int>(root->type()),
                static_cast<void*>(root));
#endif

        Marker marker;
        marker.visit(root);
    }

    void Heap::sweep() {
#ifdef CPPLOX_DEBUG_LOG_GC
        const std::uint64_t before = bytesAllocated;
#endif

        Object* prev = nullptr;
        Object* current = objects;
        while (current != nullptr) {
            Object* next = current->nextObject;

            if (current->isReachable == false) {
                bytesAllocated -= current->allocSize();
                memory::freeObject(current);

                if (prev == nullptr) {
                    objects = next;
                }
                else {
                    prev->nextObject = next;
                }
            }
            else {
                current->isReachable = false;
                prev = current;
            }

            current = next;
        }

        constexpr std::uint64_t HEAP_GROWTH_FACTOR = 2;
        nextGC = bytesAllocated * HEAP_GROWTH_FACTOR;

#ifdef CPPLOX_DEBUG_LOG_GC
        println("GC Collected {} bytes from the total {}. Next run at {}.",
                before - bytesAllocated,
                before,
                nextGC);
#endif
    }

    void Heap::link(Object* obj) {
        obj->updateAllocSize(SizeKey{});
        bytesAllocated += obj->allocSize();
        obj->nextObject = objects;
        objects = obj;
    }
} // namespace cpplox
