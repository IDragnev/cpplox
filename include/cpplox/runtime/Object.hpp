#pragma once

#include "cpplox/runtime/GCVisitor.hpp"

#include <cstddef>

namespace cpplox {
    enum class ObjectType {
        FUNCTION,
        CLOSURE,
        UPVALUE,
        CLASS,
        INSTANCE,
        BOUND_METHOD,
        NATIVE_FUNCTION,
        LIST,
    };

    template <typename T>
    concept HasTypeTag = requires { T::TYPE; };

    class Heap;

    // Only the heap may change an object's alloc size, because it must change
    // together with the heap's byte count.
    class SizeKey {
        friend class Heap;
        SizeKey() = default;
    };

    class Object {
    public:
        explicit Object(ObjectType t) : _type(t) {}
        virtual ~Object() = default;

        virtual void trace(gc::Visitor& v) = 0;

        ObjectType type() const { return _type; }
        bool hasType(ObjectType t) const { return _type == t; }
        // Alloc size as last accounted by the owning heap, 0 until owned.
        std::size_t allocSize() const { return _allocSize; }
        void updateAllocSize(SizeKey) { _allocSize = computeAllocSize(); }

        template <HasTypeTag T>
        T* as() {
            return _type == T::TYPE ? static_cast<T*>(this) : nullptr;
        }
        template <HasTypeTag T>
        const T* as() const {
            return _type == T::TYPE ? static_cast<const T*>(this) : nullptr;
        }

    public:
        bool isReachable = false;
        Object* nextObject = nullptr;

    protected:
        // Heap bytes this object is responsible for: the object itself plus
        // the buffers it owns. Embedded containers count only their buffers
        // (getAllocSize()), since the object's sizeof already covers them.
        virtual std::size_t computeAllocSize() const = 0;

    private:
        ObjectType _type;
        std::size_t _allocSize = 0;
    };
} // namespace cpplox
