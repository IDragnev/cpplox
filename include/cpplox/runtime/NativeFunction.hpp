#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/core/Value.hpp"

#include <span>

namespace cpplox {
    class Arity {
    public:
        static Arity exactly(unsigned count) { return Arity(count, false); }
        static Arity any() { return Arity(0, true); }

        bool accepts(unsigned argc) const { return _any || argc == _exact; }
        bool isAny() const { return _any; }
        unsigned exact() const { return _exact; }

    private:
        Arity(unsigned exact, bool any) : _exact(exact), _any(any) {}

        unsigned _exact;
        bool _any;
    };

    class NativeFunction : public Object {
    public:
        static constexpr ObjectType TYPE = ObjectType::NATIVE_FUNCTION;

        NativeFunction(const String& name, Arity arity);

        void trace(gc::Visitor& v) override;

        // Returns false on failure. A failing native may leave a string in
        // result, which the VM reports as the runtime error message.
        virtual bool call(std::span<Value> args, Value& result) = 0;

        const String name;
        const Arity arity;
    };
}
