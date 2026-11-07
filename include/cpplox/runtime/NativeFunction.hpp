#pragma once

#include "cpplox/runtime/Object.hpp"
#include "cpplox/runtime/RuntimeStr.hpp"
#include "cpplox/core/Value.hpp"

#include <span>

namespace cpplox {
    class NativeContext;

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

        NativeFunction(RuntimeStr* name, Arity arity);

        void trace(gc::Visitor& v) override;

        // On failure, report through ctx (error() or a failed allocation); the
        // VM then discards the returned value.
        virtual Value call(NativeContext& ctx, std::span<Value> args) = 0;

        RuntimeStr* const name = nullptr;
        const Arity arity;

    protected:
        std::size_t computeAllocSize() const override;
    };
}
