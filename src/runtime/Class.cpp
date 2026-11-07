#include "cpplox/runtime/Class.hpp"

namespace cpplox {
    Class::Class(RuntimeStr* name)
        : Object(Class::TYPE)
        , name(name)
    {}

    void Class::trace(gc::Visitor& v) {
        v.visit(name);

        methods.forEach([&v](RuntimeStr* key, Value& val) {
            v.visit(key);
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }

    std::size_t Class::computeAllocSize() const {
        return sizeof(Class) + methods.getAllocSize();
    }
}