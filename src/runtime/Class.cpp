#include "cpplox/runtime/Class.hpp"

namespace cpplox {
    Class::Class(const String& name)
        : Object(Class::TYPE)
        , name(name)
    {}

    void Class::trace(gc::Visitor&) {}
}