#pragma once

#include "cpplox/runtime/Memory.hpp"
#include "cpplox/runtime/Object.hpp"

#include <cstdint>
#include <utility>

namespace cpplox {
    class Heap {
    public:
        explicit Heap(std::uint64_t firstCollectionAt)
            : nextGC(firstCollectionAt)
        {}
        ~Heap();

        Heap(const Heap&) = delete;
        Heap& operator=(const Heap&) = delete;

        // Never collects - the owner decides when to collect.
        template <typename T, typename... Args>
        T* make(Args&&... args) {
            T* obj = memory::makeObject<T>(std::forward<Args>(args)...);
            if (obj != nullptr) {
                link(obj);
            }

            return obj;
        }

        void adopt(Object* objects);
        void resized(Object* obj);
        std::uint64_t allocated() const { return bytesAllocated; }

        bool needsCollection() const { return bytesAllocated > nextGC; }
        void traceRoot(Object* root);
        void sweep();

    private:
        void link(Object* obj);

    private:
        Object* objects = nullptr;
        std::uint64_t bytesAllocated = 0;
        std::uint64_t nextGC = 0;
    };
} // namespace cpplox
