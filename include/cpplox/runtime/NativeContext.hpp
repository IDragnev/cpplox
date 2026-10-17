#pragma once

namespace cpplox {
    class Heap;
    class Object;

    class NativeContext {
    public:
        explicit NativeContext(Heap& heap) : heap(&heap) {}

        void resized(Object* obj);

    private:
        Heap* heap = nullptr;
    };
} // namespace cpplox
