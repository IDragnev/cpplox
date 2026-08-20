#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/core/String.hpp"
#include "cpplox/core/Value.hpp"

#include <span>

namespace cpplox {
    class Arity {
    public:
        static Arity exactly(unsigned count) { return Arity(count, count, false); }
        static Arity between(unsigned min, unsigned max) {
            return Arity(min, max, false);
        }
        static Arity any() { return Arity(0, 0, true); }

        bool accepts(unsigned argc) const {
            return _any || (argc >= _min && argc <= _max);
        }
        bool isAny() const { return _any; }
        bool isExact() const { return _any == false && _min == _max; }

        unsigned min() const { return _min; }
        unsigned max() const { return _max; }
        unsigned exact() const { return _min; }

    private:
        Arity(unsigned min, unsigned max, bool any)
            : _min(min)
            , _max(max)
            , _any(any)
        {}

        unsigned _min;
        unsigned _max;
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
