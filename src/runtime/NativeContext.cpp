#include "cpplox/runtime/NativeContext.hpp"
#include "cpplox/runtime/Heap.hpp"

namespace cpplox {
    void NativeContext::resized(Object* obj) {
        heap->resized(obj);
    }
} // namespace cpplox
