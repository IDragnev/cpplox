#pragma once

#include <cstddef>

namespace cpplox {
    class Heap;
    class Object;
    class List;

    class NativeContext {
    public:
        explicit NativeContext(Heap& heap) : heap(&heap) {}

        void resized(Object* obj);
        // Never collects; returns nullptr when out of memory.
        List* newList(std::size_t count);

    private:
        Heap* heap = nullptr;
    };
} // namespace cpplox
