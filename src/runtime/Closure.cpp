#include "cpplox/runtime/Closure.hpp"
#include "cpplox/runtime/Upvalue.hpp"
#include "cpplox/core/Algorithm.hpp"

namespace cpplox {
    Closure::Closure(Function* fun)
        : Object(TYPE)
        , function(fun)
        , upvalues(fun->upvaluesCount)
    {}

    void Closure::trace(gc::Visitor& v) {
        v.visit(function);

        forEach(upvalues, [&v](Upvalue* upv) {
            v.visit(upv);
        });
    }
} // namespace cpplox