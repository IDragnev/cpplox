#include "cpplox/runtime/Function.hpp"
#include "cpplox/core/Algorithm.hpp"

namespace cpplox {
    Function::Function(RuntimeStr* name, RuntimeStr* source)
        : Object(Function::TYPE)
        , name(name)
        , source(source)
    {}

    void Function::trace(gc::Visitor& v) {
        v.visit(name);
        v.visit(source);
        
        forEach(chunk.constants, [&v](Value& val) {
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }

    std::size_t Function::computeAllocSize() const {
        return sizeof(Function)
             + chunk.code.getAllocSize()
             + chunk.lines.getAllocSize()
             + chunk.constants.getAllocSize();
    }
} // namespace cpplox