#include "cpplox/runtime/Instance.hpp"
#include "cpplox/runtime/Class.hpp"

namespace cpplox {
    Instance::Instance(Class* c)
        : Object(Instance::TYPE)
        , klass(c)
    {}

    void Instance::trace(gc::Visitor& v) {
        v.visit(klass);

        fields.forEach([&v](RuntimeStr* key, Value& val) {
            v.visit(key);
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }

    std::size_t Instance::computeAllocSize() const {
        return sizeof(Instance) + fields.getAllocSize();
    }
} // namespace cpplox