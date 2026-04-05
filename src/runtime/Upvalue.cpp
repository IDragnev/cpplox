#include "cpplox/runtime/Upvalue.hpp"

namespace cpplox {
    void Upvalue::trace(gc::Visitor& v) {
        if (location != nullptr && location->isObject()) {
            v.visit(location->asObject());
        }
    }
}