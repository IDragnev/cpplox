#include "cpplox/runtime/GC.hpp"

namespace cpplox::gc {
    class Marker : public Visitor {
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

    void traceRoot(Object* obj) {
#ifdef CPPLOX_DEBUG_LOG_GC
        println("Start trace with root of type {} at {}",
                static_cast<int>(obj->type()),
                static_cast<void*>(obj));
#endif

        Marker marker;
        marker.visit(obj);

#ifdef CPPLOX_DEBUG_LOG_GC
        println("End trace with root at {}",
                static_cast<void*>(obj));
#endif
    }

    void freeObject(Object* obj) {
#ifdef CPPLOX_DEBUG_LOG_GC
        println("Free object of type {} at {}",
                static_cast<int>(obj->type()),
                static_cast<void*>(obj));
#endif
        delete obj;
    }
} // namespace cpplox::gc