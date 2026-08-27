#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    NativeFunction::NativeFunction(const String& name, Arity arity)
        : Object(NativeFunction::TYPE, sizeof(NativeFunction))
        , name(name)
        , arity(arity)
    {}

    void NativeFunction::trace(gc::Visitor&) {}
} // namespace cpplox
