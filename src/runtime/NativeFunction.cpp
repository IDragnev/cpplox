#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    NativeFunction::NativeFunction(const String& name, unsigned arity)
        : Object(NativeFunction::TYPE)
        , name(name)
        , arity(arity)
    {}

    void NativeFunction::trace(gc::Visitor&) {}
} // namespace cpplox
