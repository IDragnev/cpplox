#include "cpplox/runtime/Function.hpp"

namespace cpplox {
    Function::Function(const String& name)
        : Function(name, 0)
    {}

    Function::Function(const String& name, unsigned arity)
        : Function(name, arity, Chunk{})
    {}

    Function::Function(const String& name, unsigned arity, const Chunk& c)
        : Object(Function::TYPE)
        , name(name)
        , arity(arity)
        , chunk(c)
    {}
} // namespace cpplox