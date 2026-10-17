#include "cpplox/runtime/NativeContext.hpp"
#include "cpplox/runtime/Heap.hpp"
#include "cpplox/runtime/List.hpp"

namespace cpplox {
    void NativeContext::resized(Object* obj) {
        heap->resized(obj);
    }

    List* NativeContext::newList(std::size_t count) {
        return heap->make<List>(count);
    }
} // namespace cpplox
