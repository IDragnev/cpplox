#include "cpplox/runtime/Function.hpp"

namespace cpplox {
    Function::Function(const String& name)
        : Object(Function::TYPE)
        , name(name)
    {}
} // namespace cpplox