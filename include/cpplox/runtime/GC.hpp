#pragma once

#include "cpplox/runtime/Object.hpp"
#ifdef CPPLOX_DEBUG_LOG_GC
    #include "cpplox/log/Log.hpp"
#endif

#include <utility>

namespace cpplox::gc {
    template <typename T, typename... Args>
    inline T* makeObject(Args&&... args) {
        T* obj = new (std::nothrow) T(std::forward<Args>(args)...);

#ifdef CPPLOX_DEBUG_LOG_GC
        if (obj != nullptr) {
            println("Allocated object of type {} at {}",
                    static_cast<int>(obj->type()),
                    static_cast<void*>(obj));
        }
        else {
            println("Allocation failed!");
        }
#endif

        return obj;
    }

    void traceRoot(Object* root);
    void freeObject(Object* obj);
} // namespace cpplox::gc