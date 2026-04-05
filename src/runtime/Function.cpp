#include "cpplox/runtime/Function.hpp"
#include "cpplox/core/Algorithm.hpp"

namespace cpplox {
    Function::Function(const String& name)
        : Object(Function::TYPE)
        , name(name)
    {}

    void Function::trace(gc::Visitor& v) {
        forEach(chunk.constants, [&v](Value& val) {
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }
} // namespace cpplox