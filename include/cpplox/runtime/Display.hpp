#pragma once

#include "cpplox/core/String.hpp"
#include "cpplox/core/Value.hpp"

namespace cpplox {
    String display(const Value& v);
    String str(const Value& v);
} // namespace cpplox
