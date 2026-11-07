#include "cpplox/runtime/NativeFunction.hpp"

namespace cpplox {
    NativeFunction::NativeFunction(RuntimeStr* name, Arity arity)
        : Object(NativeFunction::TYPE)
        , name(name)
        , arity(arity)
    {}

    void NativeFunction::trace(gc::Visitor& v) {
        v.visit(name);
    }

    std::size_t NativeFunction::computeAllocSize() const {
        return sizeof(NativeFunction);
    }
} // namespace cpplox
