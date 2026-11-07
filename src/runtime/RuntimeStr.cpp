#include "cpplox/runtime/RuntimeStr.hpp"

#include <utility>

namespace cpplox {
    RuntimeStr::RuntimeStr(std::string_view sv)
        : Object(RuntimeStr::TYPE)
        , str(sv)
    {}

    RuntimeStr::RuntimeStr(String s)
        : Object(RuntimeStr::TYPE)
        , str(std::move(s))
    {}

    void RuntimeStr::trace(gc::Visitor&) {}

    std::size_t RuntimeStr::computeAllocSize() const {
        return sizeof(RuntimeStr) + str.capacity();
    }
}
