#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"

#include <string_view>

namespace cpplox {
    class RuntimeStr : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::STRING;

        explicit RuntimeStr(std::string_view sv);
        explicit RuntimeStr(String s);

        void trace(gc::Visitor& v) override;

        String str;

    protected:
        std::size_t computeAllocSize() const override;
    };
}
