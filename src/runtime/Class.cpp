#include "cpplox/runtime/Class.hpp"

namespace cpplox {
    Class::Class(const String& name)
        : Object(Class::TYPE, sizeof(Class))
        , name(name)
    {}

    void Class::trace(gc::Visitor& v) {
        methods.forEachValue([&v](Value& val) {
            if (val.isObject()) {
                v.visit(val.asObject());
            }
        });
    }
}