#include "cpplox/core/Value.hpp"
#include <stdio.h>

namespace cpplox {
    void printValue(const Value& v) {
        printf("%g", v);
    }
}