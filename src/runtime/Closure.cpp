#include "cpplox/runtime/Closure.hpp"

namespace cpplox {
    Closure::Closure(const Function* fun)
        : Object(TYPE)
        , function(fun)
    {}
} // namespace cpplox