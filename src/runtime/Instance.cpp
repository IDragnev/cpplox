#include "cpplox/runtime/Instance.hpp"
#include "cpplox/runtime/Class.hpp"

namespace cpplox {
    Instance::Instance(Class* c)
        : Object(Instance::TYPE, sizeof(Instance))
        , klass(c)
    {}

    void Instance::trace(gc::Visitor& v) {
        v.visit(klass);

        fields.forEachValue([&v](Value& val) {
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }
} // namespace cpplox