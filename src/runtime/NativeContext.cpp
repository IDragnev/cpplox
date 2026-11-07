#include "cpplox/runtime/NativeContext.hpp"
#include "cpplox/runtime/Heap.hpp"
#include "cpplox/runtime/List.hpp"
#include "cpplox/runtime/RuntimeStr.hpp"

namespace cpplox {
    void NativeContext::resized(Object* obj) {
        heap->resized(obj);
    }

    List* NativeContext::newList(std::size_t count) {
        return make<List>(count);
    }

    RuntimeStr* NativeContext::newString(String str) {
        return make<RuntimeStr>(std::move(str));
    }

    template <typename T, typename... Args>
    T* NativeContext::make(Args&&... args) {
        T* obj = heap->make<T>(std::forward<Args>(args)...);
        if (obj == nullptr) {
            outOfMemory();
        }

        return obj;
    }
} // namespace cpplox
