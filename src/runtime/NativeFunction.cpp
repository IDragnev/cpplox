#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    NativeFunction::NativeFunction(const String& name, Arity arity)
        : Object(NativeFunction::TYPE)
        , name(name)
        , arity(arity)
    {}

    void NativeFunction::trace(gc::Visitor&) {}

    std::size_t NativeFunction::computeAllocSize() const {
        return sizeof(NativeFunction);
    }
} // namespace cpplox
