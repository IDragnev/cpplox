#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/core/Value.hpp"

#include <span>

namespace cpplox {
    class NativeFunction : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::NATIVE_FUNCTION;

        NativeFunction(const String& name, unsigned arity);

        void trace(gc::Visitor& v) override;

        // Returns false on failure. A failing native may leave a string in
        // result, which the VM reports as the runtime error message.
        virtual bool call(std::span<Value> args, Value& result) = 0;

        const String name;
        const unsigned arity;
    };
}
