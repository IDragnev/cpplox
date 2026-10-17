#include "cpplox/runtime/Memory.hpp"

namespace cpplox::memory {
    void freeObject(Object* obj) {
#ifdef CPPLOX_DEBUG_LOG_GC
        println("Free object of type {} at {}",
                static_cast<int>(obj->type()),
                static_cast<void*>(obj));
#endif
        delete obj;
    }
} // namespace cpplox::memory
