#include "cpplox/runtime/BoundMethod.hpp"
#include "cpplox/runtime/Closure.hpp"

namespace cpplox {
    BoundMethod::BoundMethod(Value r, Closure* m)
        : Object(BoundMethod::TYPE, sizeof(BoundMethod))
        , receiver(r)
        , method(m) {}

    void BoundMethod::trace(gc::Visitor& v) {
        v.visit(method);

        if (receiver.isObject()) {
            v.visit(receiver.asObject());
        }
    }
} // namespace cpplox